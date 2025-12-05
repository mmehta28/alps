import simulate
import workload
import algorithm
import time
import ebpf
from cpu_time import trace_print, update_cpuT
import threading
import struct
import socket
import psutil
import argparse

SERVER_IP = '127.0.0.1'
SERVER_PORT = 8083
STRUCT_FORMAT = '<100Q'

def unpack_policy(message):
    policy = list(message)
    processed_policy = {}
    for i in range(len(message)):
        priority = message[i] % 1000000
        ts = int((message[i] - priority)/1000000)
        processed_policy[i+1] = [priority, ts]
    return processed_policy

def pack_policy(new_policy):
    packed_policy = []
    # ALPS Protocol requires exactly 100 items (Classes 1 to 100)
    for k in range(1, 101):
        if k in new_policy:
            # Existing policy from algorithm
            # Format: Timeslice * 1,000,000 + Priority
            priority = int(new_policy[k][0])
            timeslice = int(new_policy[k][1])
            v = timeslice * 1000000 + priority
        else:
            # Default for classes not in the workload
            # Priority 0, Timeslice 10ms
            # (10 * 1,000,000 + 0) = 10,000,000
            v = 10 * 1000000 + 0
            
        packed_policy.append(v)
        
    return [int(x) for x in packed_policy]

def handle_client(client_socket, args):
    init = True
    init_v = 0
    
    # [Closed Loop] Track the last policy we sent to use as 'old_policy' 
    # for smoothing in algorithm.py, since the client no longer echoes it back.
    last_sent_policy = {} 

    while True:
        # 1. Receive Data (Blocking)
        data = client_socket.recv(struct.calcsize(STRUCT_FORMAT))
        if not data:
            break
        
        # 2. CPU Utilization Logic (Preserved)
        time.sleep(1) 
        per_cpu_utilization = psutil.cpu_percent(interval=1, percpu=True)[:24]
        cpu_ulilization = sum(per_cpu_utilization) / len(per_cpu_utilization)
        
        # 3. Unpack Stats (New Logic)
        # The buffer contains 100 integers representing avg_wait_ns for each class
        # We convert this tuple into a dictionary {class_id: wait_time_ns}
        ghost_stats_raw = struct.unpack(STRUCT_FORMAT, data)
        ghost_stats = {i+1: ghost_stats_raw[i] for i in range(100)}
        
        # 4. Create Workload with Feedback
        # Pass the real stats (ghost_stats) so the simulation reflects reality
        wl, init_v = workload.readWorkload(init, init_v, ghost_stats)
        init = False
        
        # 5. Run Simulation
        simulate.simulate(wl, "srtf", "", timeSlice = 8, period = 2000, CScost = 0)
        
        # 6. Run Machine Learning Algorithm
        # We pass 'last_sent_policy' instead of 'previous' (which used to be the echo).
        if args.ml == "LR":
            new = algorithm.LinerRegression(cpu_ulilization, args, last_sent_policy)
        elif args.ml == "RF":
            new = algorithm.RandomForest(cpu_ulilization, args, last_sent_policy)
        elif args.ml == "EWMV":
            new = algorithm.ExponentialWeightedMovingAverage(cpu_ulilization, args, last_sent_policy)
        else:
            new = algorithm.heurtistic(cpu_ulilization, args, last_sent_policy)
            
        # [Closed Loop] Update history for next iteration
        last_sent_policy = new

        # 7. Pack and Send Response
        packed_new = pack_policy(new)
        response = struct.pack(STRUCT_FORMAT, *packed_new)
        client_socket.send(response)

    client_socket.close()

def main():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((SERVER_IP, SERVER_PORT))
    server_socket.listen()
    parser = argparse.ArgumentParser(description="ALPS: Adaptive-learning Priority OS scheduler for Serverless Functions")
    parser.add_argument("--alpha", type=float, help="alpha", default = 1)
    parser.add_argument("--beta", type=float, help="beta", default = 1)
    parser.add_argument("--gamma", type=float, help="gamma", default = 1)
    parser.add_argument("--theta", type=float, help="theta", default = 50)
    parser.add_argument("--exp_cpu", type=str, help="CPU profile", default = "/mydata/exp_cpu/test1")
    parser.add_argument("--exp_result", type=str, help="experiment", default = "../experiments/seals")
    parser.add_argument("--ml", type=str, help="Machine Learning algorithms", default="avg")
    parser.add_argument("--unpred", action='store_false', help="Unpreditability fine-tune")
    parser.add_argument("--overload", action='store_false', help="Overload fine-tune")
    args = parser.parse_args()
    try:
        if args.exp_result != "":
            stop_event = threading.Event()
            thread = threading.Thread(target=trace_print, args=(stop_event,args.exp_cpu,))
            thread.start()
        while True:
            client_socket, addr = server_socket.accept()
            handle_client(client_socket, args)
    except KeyboardInterrupt:
        print("Server is shutting down.")
    finally:
        if args.exp_result != "":
            stop_event.set()
            thread.join()
            update_cpuT(args.exp_cpu, args.exp_result)
        client_socket.close()
        server_socket.close()


if __name__ == "__main__":
    main()
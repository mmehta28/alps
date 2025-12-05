import subprocess
import time
import os
import sys

# Usage: python3 run_alps_test.py <fib_input> <alps_class_id>
# alps_class_id: 0 (Short/8ms), 1 (Long/200ms), 2 (Medium/100ms)

def run_experiment(fib_n, class_id):
    # 1. Start the workload in the background
    proc = subprocess.Popen(
        ["./fib_alps", str(fib_n)], 
        stdout=subprocess.PIPE,
        text=True
    )

    # 2. Read the PID from the first line of output
    pid_line = proc.stdout.readline()
    if not pid_line.startswith("PID:"):
        print("Error starting workload")
        return
    
    pid = int(pid_line.strip().split(":")[1])
    print(f"Launched PID {pid} -> Assigning ALPS Class {class_id}")

    # 3. [The ALPS Logic] Write the policy to the side-channel
    # The C++ agent reads this file to determine priority and time slice
    with open(f"/tidinfo/{pid}.txt", "w") as f:
        f.write(str(class_id))

    # 4. Move process to ghOSt (Enclave 1 is standard for agent_cfs)
    # We use sudo tee to write to the sysfs file
    subprocess.run(f"echo {pid} | sudo tee /sys/fs/ghost/enclave_1/tasks > /dev/null", shell=True)

    # 5. Wait for finish and capture result
    output, _ = proc.communicate()
    print(output.strip())

    # Cleanup the info file
    if os.path.exists(f"/tidinfo/{pid}.txt"):
        os.remove(f"/tidinfo/{pid}.txt")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 run_alps_test.py <fib_num> <class_id>")
        sys.exit(1)
    
    run_experiment(sys.argv[1], sys.argv[2])

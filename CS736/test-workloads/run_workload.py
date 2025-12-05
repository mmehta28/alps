import time
import subprocess
import csv
import threading
import sys

def run_function(func_id, duration):
    # This launches the process. 
    # If running on ghOSt, the ghOSt agent will automatically pick this up 
    # if you launch this script inside the ghOSt enclave or attach it.
    subprocess.run(["./fib_task", str(duration)])

def main():
    start_time = time.time() * 1000 # Convert to ms
    
    with open('azure_trace.csv', 'r') as f:
        reader = csv.reader(f)
        next(reader) # Skip header
        
        for row in reader:
            if not row or row[0].startswith('#'): continue
            
            arrival_time = int(row[0])
            func_id = int(row[1])
            duration = int(row[2])
            
            # Wait until it's time to launch (Arrival Time)
            current_time = time.time() * 1000
            elapsed = current_time - start_time
            
            if arrival_time > elapsed:
                time.sleep((arrival_time - elapsed) / 1000.0)
            
            # Spawn the function in a separate thread (simulating concurrency)
            t = threading.Thread(target=run_function, args=(func_id, duration))
            t.start()
            print(f"Launched Func {func_id} for {duration}ms")

if __name__ == "__main__":
    main()

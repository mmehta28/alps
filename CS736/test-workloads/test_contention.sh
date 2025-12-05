#!/bin/bash

# 1. Clean up previous noise if any
pkill noise

echo "--- Step 1: Launching Background Noise (Class 2) ---"
./noise &
NOISE_PID=$!
# Give it a moment to start
sleep 0.1

# Assign Noise to ALPS Class 2 (Medium Slice)
echo "2" > /tidinfo/$NOISE_PID.txt
# Move Noise to ghOSt enclave
echo $NOISE_PID | sudo tee /sys/fs/ghost/enclave_1/tasks > /dev/null
echo "Noise running on PID $NOISE_PID (Class 2)"
echo "----------------------------------------------------"

echo "--- Step 2: Running Fib Task as Class 1 (Long Slice / High Prio) ---"
# We expect this to be FASTER because it holds the CPU longer (200ms)
# and bullies the noise task.
python3 run_alps_test.py 44 1

echo "----------------------------------------------------"

echo "--- Step 3: Running Fib Task as Class 0 (Short Slice / Low Prio) ---"
# We expect this to be SLOWER because it yields every 8ms,
# giving the Noise task more chances to run.
python3 run_alps_test.py 44 0

# Cleanup
echo "----------------------------------------------------"
echo "Cleaning up..."
kill $NOISE_PID
rm /tidinfo/$NOISE_PID.txt

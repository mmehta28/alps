#!/bin/bash

# Clean up
pkill noise

echo "============================================================"
echo "TEST 1: High Priority Contention (Sticky CPU)"
echo "Both tasks are Class 1 (200ms Slice). They should switch RARELY."
echo "============================================================"

# 1. Start Noise as Class 1 (Equal competitor)
./noise &
NOISE_PID=$!
sleep 0.1
echo "1" > /tidinfo/$NOISE_PID.txt
echo $NOISE_PID | sudo tee /sys/fs/ghost/enclave_1/tasks > /dev/null

# 2. Run Fib as Class 1
python3 run_alps_test.py 40 1

# Clean up noise for next test
kill $NOISE_PID
rm /tidinfo/$NOISE_PID.txt
sleep 1

echo ""
echo "============================================================"
echo "TEST 2: Low Priority Contention (Thrashing)"
echo "Both tasks are Class 0 (8ms Slice). They should switch FREQUENTLY."
echo "============================================================"

# 1. Start Noise as Class 0 (Equal competitor)
./noise &
NOISE_PID=$!
sleep 0.1
echo "0" > /tidinfo/$NOISE_PID.txt
echo $NOISE_PID | sudo tee /sys/fs/ghost/enclave_1/tasks > /dev/null

# 2. Run Fib as Class 0
python3 run_alps_test.py 40 0

# Cleanup
kill $NOISE_PID
rm /tidinfo/$NOISE_PID.txt

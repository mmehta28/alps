import job
import random

# Mock get_cpu_time to return a dummy duration (e.g., 100ms)
def get_cpu_time(cid):
    return 100 

# Updated readWorkload to accept real_stats from Ghost
def readWorkload(init, init_v, real_stats=None, lens=100):
    wl = []
    
    # Generate 'lens' number of dummy jobs
    for i in range(lens):
        invocationId = init_v + i + 1
        startTime = i * 10  # Stagger arrival times
        class_id = (i % 4) + 1  # Rotate through class IDs 1-4
        
        # Create a job with dummy parameters
        # Job signature: (id, start_time, execution_time, priority, arrival_index, class_id)
        new_job = job.Job(invocationId, startTime, get_cpu_time(class_id), class_id, invocationId, class_id)
        
        # [ALPS CLOSED LOOP INTEGRATION]
        # If we received real statistics from the Ghost agent (wait times in ns),
        # inject them into this job. This tricks the ALPS simulation/algorithm into 
        # optimizing for the *real* latency happening in the kernel.
        if real_stats and real_stats.get(class_id, 0) > 0:
            # Ghost sends nanoseconds. ALPS typically operates in milliseconds.
            real_wait_ms = real_stats[class_id] / 1000000.0
            new_job.waitTime = real_wait_ms
            
        wl.append(new_job)

    # Update init_v for the next batch
    new_init_v = init_v + lens
    return wl, new_init_v
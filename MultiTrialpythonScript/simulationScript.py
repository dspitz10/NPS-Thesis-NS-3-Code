import os
import subprocess
import time
import random

# List of bad seeds
bad_seeds = {87275, 50287, 16761, 42520, 17771, 45588, 52754, 44245,128903,623600,290405,343341,248572,562758,650399,167742,620278,127893,719782,583498,845536,214754}

def outPutSeedInfo(badSeeds):
    # Determine the desktop path
    desktop_path = os.path.join(os.path.expanduser("~"), "Desktop")
    file_path = os.path.join(desktop_path, "bad_seeds.txt")

    # Write the bad seeds to a text file
    with open(file_path, 'w') as f:
        for runNum, seed in badSeeds.items():
            f.write(f"Run Number {runNum}, bad seed: {seed}\n")

    print(f"Bad seeds saved to {file_path}")

def get_random_seed():
    while True:
        seed = random.randint(0, 1000000)  # Adjust range as needed
        if seed not in bad_seeds:
            return seed

def run_ns3_simulation(script_name, num_runs, satSetup, time_limit,errorSet,runStart):
    totalTime = 0
    runNum = runStart #for LEO 10% error rate
    encounteredBadSeed = []

    badSeedInfo = {}
    seedReset = get_random_seed()
    seed_value  = runNum + 15000

    for i in range(runNum, num_runs+1):
        retries = 0
        sucess = False
        # print(f"Got here run {runNum}")
        # seed_value = random.randint(1, 100000)
        while retries < 5 and not sucess:
            # if abs(seed_value - 1000000) < 256:
            #     seed_value = seedReset
            # seed_value =  get_random_seed()
            command = f'./ns3 run "{script_name} --RngRun={seed_value} --runNumber={runNum} --satSetup={satSetup} --errorRate={errorSet}"'
            print(f"Running simulation Run Number {runNum} with seed: {seed_value}")
            
            # Measure time for each run
            start_time = time.time()

            try:
                # Execute the command with a timeout
                process = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=time_limit)
                stdout, stderr = process.stdout, process.stderr

                # Calculate elapsed time for this run
                elapsed_time = time.time() - start_time
                totalTime += elapsed_time

                # Output the results
                if process.returncode == 0:
                    print(f"Run {runNum} completed successfully.")
                    print(stdout.decode())
                    sucess = True
                    runNum += 1
                    seed_value += 1

                else:
                    print(f"Run {runNum} for {seed_value} encountered an error. Retrying...")
                    print(stderr.decode())

            except subprocess.TimeoutExpired:
                print(f"Run {runNum} timed out after {time_limit} seconds.")
                seed_value += 1
                badSeedInfo[runNum] = seed_value
                retries += 1
                if retries == 25:
                    print(f"Increasing Seed Value by Random Number")
                    seed_value = get_random_seed()
                # badRuns.append(runNum)
                # encounteredBadSeed.append(seed_value)
                continue

    print(f"Elapsed Time for Run: {runNum}: {totalTime}")
    print(f"Bad Seeds: {badSeedInfo}")

    outPutSeedInfo(badSeedInfo)

if __name__ == "__main__":
    # Replace 'your_script_name' with the ns-3 script you want to run
    # run_ns3_simulation('scratch/OnOff-TCP-multiNode.cc', 10)  # Run X times with different seeds
    RUNS = 100 # Number of runs
    simulation = ['scratch/nrtv-baseline-udp.cc','scratch/updated_three-node-OnOff-test.cc', 'scratch/v2fileDownload.cc'] #'scratch/nrtv-baseline-udp.cc', 
    combinedSimulation = 'scratch/OnOff-TCP-multiNode.cc'
    satSetup = "GEO"
    time_limit = 30 # seconds
    errorSet = [0.001, 0.050, 0.100]
    # for i in range(len(simulation)):
    #     run_ns3_simulation(simulation[i],100,satSetup,time_limit)  # Run X times with different seedss
    # run_ns3_simulation(combinedSimulation,RUNS,satSetup,time_limit)  # Run X times with different seedss
    # print(f"Finished running {combinedSimulation} {RUNS} times.")

    
    # run_ns3_simulation(combinedSimulation,RUNS,satSetup,time_limit,errorSet[0],runStart)  # Run X times with different seedss


    #iterate over error set

    # for i,error in errorSet:
    #     run_ns3_simulation(combinedSimulation,RUNS,satSetup,time_limit,error)  # Run X times with different seedss
    #     run_ns3_simulation(simulation[i],RUNS,satSetup,time_limit,error)  # Run X times with different seedss

    # for i, error in errorSet:
    #     run_ns3_simulation(combinedSimulation,RUNS,"GEO",time_limit,error)
    #     run_ns3_simulation(simulation[i],RUNS,"GEO",time_limit,error) 


    # runStart = 1000
    # run_ns3_simulation(combinedSimulation, 1001, "GEO", time_limit, errorSet[0],runStart)


    runStart = 929 # start the run logging at runStart
    # for i in range(len(simulation)):
    # run_ns3_simulation(simulation[0], 1000, "LEO", time_limit, errorSet[0],runStart)
    # run_ns3_simulation(simulation[0], 1000, "LEO", time_limit, errorSet[1],runStart)
    run_ns3_simulation(simulation[0], 1000, "LEO", time_limit, errorSet[2],runStart)   

    # runStart = 3
    # # ------------------------------------------------------------------------------------
    # run_ns3_simulation(simulation[2], 14, "LEO", time_limit, errorSet[2],runStart)
    # runStart = 995
    # run_ns3_simulation(simulation[2], RUNS, "LEO", time_limit, errorSet[2],runStart)
    #-----------------------------------------------------------------------------------

    # simulation = ['scratch/nrtv-baseline-udp','scratch/three-node-OnOff-test.cc', 'scratch/v2fileDownload.cc']
    # runStart = 1

   

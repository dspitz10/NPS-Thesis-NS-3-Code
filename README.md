
# NPS Thesis NS-3 Code

This repository contains the code used to run my NS-3 simulations for my thesis. It includes guidance on how to install and initialize the NS-3 environment on your local machine and how to run the different parts of the code.

## Start Here
Reference the Getting Started NS-3 Documentation for required software prerequisites for successful NS-3 installation and execution: [NS-3 Getting Started Documentation](https://www.nsnam.org/docs/tutorial/html/getting-started.html)

## NS-3 Installation
For Linux-based systems, this video: [NS-3 Installation Tutorial](https://youtu.be/3tDyylZsPy4), provides a streamlined way to install `ns-3-dev` into your home folder. It covers steps for switching to different releases and downloading additional modules in the `contrib` folder. macOS systems should work with minimal differences in the installation process. Other tutorials may refer to the bake command as a different way to install NS-3, I have found that utilizing ns-3-dev with cmake as shown in the video provides more control, and is relatively straigtforward with small amounts of required command line skill. 

### 5G Lena Installation Video
[Same link as above](https://youtu.be/3tDyylZsPy4)

### 5G Lena Readme Documentation
[5G Lena Documentation](https://gitlab.com/cttc-lena/nr/-/blob/master/README.md?ref_type=heads)

### For Windows, refer to the [NS-3 Installation Documentation](https://www.nsnam.org/docs/installation/html/windows.html).

For specific IDE integration, such as VS Code or Eclipse, view the Getting Started Tutorial, Part 4: [NS-3 IDE Integration](https://www.nsnam.org/docs/tutorial/html/getting-started.html).

Helpful VS Code video: https://youtu.be/Ab8eYHhT5I8?si=JKJnQQjyTy5B7v7e

Once installed and built, ensure the traffic module is downloaded. The following command should be executed inside the `contrib` folder of `ns-3-dev`.

### Cloning the Traffic Repository

Run the following command inside the `ns-3-dev/contrib` folder:

```bash
git clone https://github.com/sns3/traffic.git traffic
```
## Helpful Tutorials
If you would like some helpful exposure to NS-3 prior to running the below simulations, this link: https://www.nsnam.org/docs/release/3.43/tutorial/ns-3-tutorial.pdf provides an NS-3 tutorial that is very in depth. 

It covers
- Prerequiste software
- Installation
- Build configurations
- NS-3 configurations
- Debugging and built in NS-3 Logging
- IDE installation.
- NS-3 Python implementations instead of C++

It can be easy to get lost in the wealth of information in the NS-3 documentation, and not really know what you are looking for until later. I would reccomend after successful installation, you work your way through this part of the documentation that covers key conceptual abstractions and concepts: https://www.nsnam.org/docs/tutorial/html/conceptual-overview.html#key-abstractions 

If you get stuck, you have multiple options: 
- YouTube has lots of help videos
  -  https://youtu.be/o_2oi1lpKtg?si=7bczgYNNBPBAaNe2
  -  https://www.youtube.com/watch?v=SFdEvhxVpnI&list=PLkruFy_9uWi1v3kvhQQ90ad6IFGHDuCeR
 
- Lastly, there is a GoogleGroups page that is active and responds to requests for help. Try to dig through what has already been asked, your question has most likely already been answered.
  - https://groups.google.com/forum/?fromgroups#!forum/ns-3-users
 
## Download Simulation Files
Once all the required code modules are installed and built, download the files for the respective simulations. I recommend downloading them to the `scratch` folder and running them from there.

### NRTV - Application One
Near Real-Time Viewing single application simulation for both GEO and LEO configurations. This script, along with all the simulation scripts, is ready to be run in a multi-trial environment with the `RngRun` value altered using a Python script executed from the terminal. The files also include the option to change the `RngRun` value for a single run of the file when executed without the Python script. This is handled with the `-- --RngRun=<insert_value>` command-line argument.

Once downloaded, run the file from the `/ns-3-dev` folder, which contains a `.waf` command named `ns3`. This allows you to run the file. You will need to specify the path to where the `.cc` file is stored.

```bash
./ns3 run scratch/nrtv-baseline-udp.cc 
```

If you wish to manipulate parameters while executing the file using the `./ns3` command rather than the Python script, use the `-- --<parameter>=<value>` syntax. An example is shown below:

```bash
./ns3 run scratch/nrtv-baseline-udp.cc -- --satSetup="LEO"
```

This forces the simulation to follow the LEO configuration set within the `nrtv-baseline-udp` code.

### PCAP File Logging

Ensure your file system path has been updated in the places where the program saves files. The compiler will indicate where errors occur if paths are incorrect. Update the output folder to somewhere outside the `ns-3-dev` directory. 

### **IMPORTANT:**
Set the logging output path to a location outside the `ns-3-dev` directory to avoid **significant slowdowns** during simulations, especially with a high number of simulation iterations!

## Running the Simulation with the Python Script
Download the multi-trial python script. Keeping it in the `ns-3-dev` folder works well, the function 'run_ns3_simulation' will execute the multi trial simulation run with the parameters you have provided.  In the 'run_ns3_simulation' function, pay attention to the 'script_name' variable. 

```python
command = f'./ns3 run "{script_name} --RngRun={seed_value} --runNumber={runNum} --satSetup={satSetup} --errorRate={errorSet}"'
print(f"Running simulation Run Number {runNum} with seed: {seed_value}")
```

Ensure the `script_name` string includes the correct directory where the file is placed. If it is in the `scratch` folder, the following should work without changes as long as the `simulation` string array remains:

```python
simulation = ['scratch/nrtv-baseline-udp.cc', 'scratch/updated_three-node-OnOff-test.cc', 'scratch/v2fileDownload.cc']
```

You can alter the python script to run how you would like it to run a specifc number of iterations in a certain order. I usually use a for loop to iterate through the 'simulation' array of folders, either with another loop to iterate through the error rates, or at a single error rate. 

### Telemetry Data - Application Two
C++ NS-3 file is found in `TelemetryData-App2` folder. 
The telemetry data file functions the same way as the NRTV application file, ensure logging output paths are set correctly for your filesystem. 

### File Transfer -Application Three
C++ NS-3 file is found in `FileTransfer-App3` folder. 
The file transfer file functions the same way as the NRTV and telemetry data simulation files. Ensure loggin output files are set to the correct path for your filesystem. 

### Multi-Application
The multi applcation scenario combines all of the files so that the applications run the same as they do in their respective individual simulation scenarios simultaneously. 

My approach to separating the data collected from the single vs. multi-application scenarios was to define the individual simulations as the "baseline" data, and the combined with no modifier. So baselinePcapData becomes pcapData, baselineCsvData becomes csvData and so on. Use whatever makes sense to you, but ensure you separate the resulting .pcap file collection.  

## Processing PCAP Files

To process the generated .pcap files, utilize the python scripts within the `pcapProcessorFiles` folder. I ended up making several copies of this file and running them simultaneously from different terminal windows to parallelize my processing efforts. You will need to specify where the .pcap files are stored, and where you want the corresponding .csv output files stored. _Store them outside the NS-3 Directory_. 

## MATLAB Scripts

The Matlab scripts are provided as a reference to build out what you need for your data visualization. They are clunky and very personalized to my data, utilize the methods within them, but I would write your own Matlab scripts to process the data how you want to. 





























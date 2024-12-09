
# NPS Thesis NS-3 Code

This repository contains the code used to run my NS-3 simulations for my thesis. It includes guidance on how to install and initialize the NS-3 environment on your local machine and how to run the different parts of the code.

## Start Here
Reference the Getting Started NS-3 Documentation for required software prerequisites for successful NS-3 installation and execution: [NS-3 Getting Started Documentation](https://www.nsnam.org/docs/tutorial/html/getting-started.html)

## NS-3 Installation
For Linux-based systems, this video: [NS-3 Installation Tutorial](https://youtu.be/3tDyylZsPy4), provides a streamlined way to install `ns-3-dev` into your home folder. It covers steps for switching to different releases and downloading additional modules in the `contrib` folder. macOS systems should work with minimal differences in the installation process.

### 5G Lena Installation Video
[Same link as above](https://youtu.be/3tDyylZsPy4)

### 5G Lena Readme Documentation
[5G Lena Documentation](https://gitlab.com/cttc-lena/nr/-/blob/master/README.md?ref_type=heads)

### For Windows, refer to the [NS-3 Installation Documentation](https://www.nsnam.org/docs/installation/html/windows.html).

For specific IDE integration, such as VS Code or Eclipse, view the Getting Started Tutorial, Part 4: [NS-3 IDE Integration](https://www.nsnam.org/docs/tutorial/html/getting-started.html).

Once installed and built, ensure the traffic module is downloaded. The following command should be executed inside the `contrib` folder of `ns-3-dev`.

### Cloning the Traffic Repository

Run the following command inside the `ns-3-dev/contrib` folder:

```bash
git clone https://github.com/sns3/traffic.git traffic
```

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
Download the multi-trial Python script. Keeping it in the `ns-3-dev` folder works well.

```python
command = f'./ns3 run "{script_name} --RngRun={seed_value} --runNumber={runNum} --satSetup={satSetup} --errorRate={errorSet}"'
print(f"Running simulation Run Number {runNum} with seed: {seed_value}")
```

Ensure the `script_name` string includes the correct directory where the file is placed. If it is in the `scratch` folder, the following should work without changes as long as the `simulation` string array remains:

```python
simulation = ['scratch/nrtv-baseline-udp.cc', 'scratch/updated_three-node-OnOff-test.cc', 'scratch/v2fileDownload.cc']
```

### Multi-Application

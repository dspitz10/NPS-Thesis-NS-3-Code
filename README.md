# NPS-Thesis-NS-3-Code

This repository contains the code used to run my NS-3 simulations for my Thesis. It will include guidance on how to install and initialize the NS-3 environment on your local machine, and how to run the different parts of the code.

## Start Here
Reference the Getting Started NS-3 Documentation for required software prereqresuites for successful NS-3 installation and execution: https://www.nsnam.org/docs/tutorial/html/getting-started.html

## NS-3 Install
For linux based systems, this video: https://youtu.be/3tDyylZsPy4, gives a streamlined way to install ns-3-dev into your home folder, and takes you through the steps of changing to different releases, and how to download additional modules inthe contrib folder. MacOs systems should work with minimal differences with this installation process. 

### 5G Lena Installation Video https://youtu.be/3tDyylZsPy4 
  Same link as above. 
### 5G Lena Readme documentation: https://gitlab.com/cttc-lena/nr/-/blob/master/README.md?ref_type=heads

For windows, refer to NS-3 documentation, Installation: https://www.nsnam.org/docs/installation/html/windows.html 

For specific IDE integration such as VS code or Eclipse, view the Getting Started tutorial part 4: https://www.nsnam.org/docs/tutorial/html/getting-started.html

Once installed and built, ensure the traffic module is downloaded. This command should work, executed inside the contrib folder of ns-3-dev. 

### Cloning Traffic Repository

Execute the following command inside the ns-3-dev/contrib folder:

```bash
git clone https://github.com/sns3/traffic.git traffic
```

## Download Simulation Files
Once all of the required code modules are installed and built, download the files for the respective simulations. I reccomend downloading them to the scratch folder, and running them from there. 

### NRTV - Application One 
Near Real Time Viewing single application simulation for both GEO and LEO configurations. This script, along with all of the simulation scripts, are ready to be run in a multi trial environment, with the RngRun value altered using a Pythong script executed from the terminal. Built into the files, is the option to change the RngRun value for a single run of the file when executing without the python script. This is handled with the -- -- RngRun = <insert_value> command line argument when running the file. 

Once downloaded, run the file _from the /ns-3-dev_ folder, which contains a .waf command named _ns3_, this allows you to run the file. You will need to specify the path to where the .cc file is stored. 

```bash
./ns3 run scratch/nrtv-baseline-udp.cc 
```
If manipulation of parameters is desired while executing the file using the ./ns3 command rather than the python script, utilize -- -- <parameter> = <value> syntax. An example is shown below

```bash
./ns3 run scratch/nrtv-baseline-udp.cc -- -- satSetup = "LEO"
```
This forces the simulation to follow the LEO configuration that is set within the nrtv-baseline-udp code. 

### PCAP File Logging. 

Ensure your own file system path has been changed in the places where the program saves files. The compiler will tell you where the error is if you miss it, and just change the output folder to somewhere outside the ns-3-dev. 
### _IMPORTANT!! - SET THE LOGGING OUTPUT PATH TO SOMEWHERE OUTSIDE THE NS-3-DEV DIRECTORY TO AVOID MONUMENTAL SLOWDOWNS WHEN RUNNING SIMULATIONS. ESPECIALLY WITH HIGH NUMBER OF SIMULATION ITERATIONS_!! 






### Multi-application





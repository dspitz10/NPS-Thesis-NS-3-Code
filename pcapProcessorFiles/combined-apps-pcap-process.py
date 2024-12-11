import os
import scapy
from scapy.all import rdpcap
import csv
from pprint import pprint
from datetime import datetime
import scipy.io
import re 

def get_run_number(filename):
    match = re.search(r'Run_(\d+)', filename)
    return int(match.group(1)) if match else float('inf')

# define function to processor pcap files
def pcapProcessor(pcap_file, test, dst_ip,run,error_rate,csvDest):
    try:
        packets = rdpcap(pcap_file)
    except scapy.error.Scapy_Exception as e:
        print(f"Error reading file {pcap_file}: {e}")
        packets = []

    time = []
    bytes = []
    droppedPackets = 0

    packet_data = 'dst_packet_data-'+ test +'_RUN-'+ str(run) + '_' + '.csv' # dst_packet_data-NRTV-app1_RUN-0_.csv

    # Specify the output directory
    output_dir = csvDest+ error_rate+ '/' + test

    # Ensure the directory exists
    os.makedirs(output_dir, exist_ok=True)

    # Full path to the CSV file
    file_path = os.path.join(output_dir, packet_data)

    printEnabled = 0
    # print(f"Printing first 10 Packets data for OnOff Test: ")
    # Iterate over the first 10 packets
    print(f"Processing Data...")
    print(f"Processing Data in File {pcap_file}...")
    for i, packet in enumerate(packets[:]):
        if packet.haslayer('IP'):
                ip_layer = packet['IP']

        if packet.haslayer('TCP'):
                transport_layer = packet['TCP']
        elif packet.haslayer('UDP'): 
            transport_layer = packet['UDP']
        elif packet.haslayer('IPv4'):
            transport_layer = packet['IPv4']

        if printEnabled:
            # packetSlice = enumerate(packets[:15])
            # for i in range(15): #print firs 15 packets
                print(f"Packet {i+1} Summary: {packet.summary()}\n")
                # print(f"  Source IP: {ip_layer.src}, Destination IP: {ip_layer.dst}")
                # print(f"  Source Port: {transport_layer.sport}, Destination Port: {transport_layer.dport}")

                # # Print the length of each packet
                # print(f"Packet {i+1} Length: {len(packet)} bytes")
                # print(f"Packet {i+1} Time: {packet.time}")

        # Check if is DST packet and Append if true 
        if ip_layer.dst == dst_ip:
            time.append(packet.time)
            bytes.append(len(packet))
            # Print the length of each packet
            if printEnabled:
                # for i in range(15):
                    print(f"Packet {i+1} Added - Length: {len(packet)} bytes")
                    print(f"Packet {i+1} Added - Time: {packet.time}")
    
    print(f"Total Packets: {len(packets)}")
    print(f"Total Dropped Packets: {droppedPackets}")
    # print(f'Packet Drop Percentage: {droppedPackets/len(packets)*100}%')

    # Write the time and bytes to a CSV file    
    with open(file_path, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        # writer.writerow(['Time', 'Bytes'])
        for t, b in zip(time, bytes):
            writer.writerow([t, b])
    print(f"Done Writing to CSV: {packet_data}")

# MAIN CODE BLOCK
if __name__ == '__main__':
    # Add your main code logic here
    # Path to the pcap file
    folder_path = '/home/drew/Thesis/pcapData/ErrRate'
    folder_pathBase = '/home/drew/Thesis/baselinePcapData/ErrRate'



    csvDest = '/home/drew/Thesis/csvData/ErrRate'
    baseline = True
    LEO = True

    if (baseline):
        folder_path = folder_pathBase
        csvDest = '/home/drew/Thesis/baselineCsvData/ErrRate'

    if (LEO):
        folder_path = '/home/drew/Thesis/pcapData/LEO/ErrRate'
        folder_pathBase = '/home/drew/Thesis/baselinePcapData/LEO/ErrRate'
        csvDest = '/home/drew/Thesis/csvData/LEO/ErrRate'

        if (baseline):
            folder_path = folder_pathBase
            csvDest = '/home/drew/Thesis/baselineCsvData/LEO/ErrRate'
    
    # Set Error Rate for the test
    error_rate = ["0.001","0.050","0.100"]
    ER = 2

    app1_test = {"testName": "NRTV-app1", "dst_ip": "10.3.1.2"} # NRTV-app1 DST IP: 10.3.1.2

    app2_test = {"testName": "telemetryData-app2", "dst_ip": "10.4.1.2"} # telemetryData-app2 DST IP: 10.4.1.2

    app3_test = {"testName": "fileTransfer-app3", "dst_ip": "10.1.1.1"} # fileTransfer-app3 DST IP: 10.1.1.1
    
    if (baseline):
        app2_test = {"testName": "telemetryData-app2", "dst_ip": "10.3.1.2"} # telemetryData-app2 - Baseline

    # Store them in the pcapData array
    pcapData = [app1_test,app2_test,app3_test]

    # Select the test to process
    selector = 2

    if (selector == 1):
        pcapData = [app1_test]
    elif (selector == 2):
        pcapData = [app2_test]
    elif (selector == 3):
        pcapData = [app3_test]
    elif (selector == 4):
        pcapData = [app2_test,app3_test]
    elif (selector == 5):
        pcapData = [app1_test,app2_test,app3_test]
    elif (selector == 6):
         pcapData = [app1_test,app3_test]
    elif (selector == 7):
        pcapData = [app1_test,app2_test]

    start_index = 941
    end_index = 999


    # Print the result
    # print(pcapData)

    parse = 1

    # i = 2; 
    for i in range(len(pcapData)):
        trial = start_index
        test_folder = folder_path + error_rate[ER] + '/' + pcapData[i]['testName']
        
        # List files and sort them based on the run number
        files = [f for f in os.listdir(test_folder) if f.endswith(".pcap") and not f.startswith("._")]
        files.sort(key=get_run_number)


        print(f"File at Start Index: {files[start_index-1]} - File at End Index {files[end_index-1]}")
        if (parse):
            for filename in files[start_index-1:end_index]: #os.listdir(folder_path+'/'+pcapData[i]['testName']):  # Assuming you're looping through files
                try:
                    pcapProcessor(test_folder+'/'+filename, pcapData[i]['testName'], pcapData[i]['dst_ip'],trial,error_rate[ER],csvDest)
                    trial += 1
                except scapy.error.Scapy_Exception as e:
                    print(f"Error reading file {filename}: {e}")
                    continue  # This skips to the next iteration of the loop
        else:
            print("Parse Not Enabled")             

    print("Done Processing PCAP Files")
pass 
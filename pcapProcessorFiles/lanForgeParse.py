import os
import scapy
from scapy.all import rdpcap
import csv
from pprint import pprint
from datetime import datetime
import scapy.error
import scipy.io
import re 

def get_run_number(filename):
    match = re.search(r'Run_(\d+)', filename)
    return int(match.group(1)) if match else float('inf')


def pcapProcessor(folder_path,fileName,run):
    try:
        print(f"Reading file {fileName} in folder {folder_path}")
        packets = rdpcap(folder_path+ '/'+fileName)
    except scapy.error.Scapy_Exception as e:
        print(f"Error reading file {fileName}: {e}")
        packets = []
    
    output_dir = folder_path + '/csvFiles'
    outputFile = 'filtered_dst_five_RUN_' + str(run) + '.csv'
    outputPath = os.path.join(output_dir, outputFile)

    # Ensure the directory exists
    os.makedirs(output_dir, exist_ok=True)

    time = []
    bytes = []
    packetNum = []
    dst_ip = '192.168.2.100'

    initial_time = None
    printEnabled = False

    print(f"Processing Data in File {folder_path}...")
    for i, packet in enumerate(packets[:]):
        if packet.haslayer('IP'):
            ip_layer = packet['IP']
        else:
            continue


        # if packet.haslayer('TCP'):
        #     transport_layer = packet['TCP']
        if packet.haslayer('UDP'): 
            transport_layer = packet['UDP']
        elif packet.haslayer('IPv4'):
            transport_layer = packet['IPv4']
    
        if initial_time is None:
            initial_time = packet.time
        
        # if crashed:
        #     endPkt = 227423
        # Check if is DST packet and Append if true 
        if ip_layer.dst == dst_ip and packet.haslayer('UDP') and transport_layer.sport == 3480:
            time.append(packet.time - initial_time)           
            bytes.append(len(packet))
            packetNum.append(i+1) 

    if printEnabled:
        for i in range(20):
            print(f"Packet {i+1} Summary: {packet.summary()}\n")
            print(f"  Source IP: {ip_layer.src}, Destination IP: {ip_layer.dst}")
            print(f"  Source Port: {transport_layer.sport}, Destination Port: {transport_layer.dport}")        
                
    if printEnabled:
        # print(f"Bytes: {bytes} Time: {time}")
        for j in range(20):
            print(f"Packet {packetNum[j]} Added - {bytes[j]} bytes - Time: {time[j]}")
    
    print(f"Processed {len(packets)} packets.")
    print(f"Check Sizes: ...\n")
    if len(bytes) == len(time) == len(packetNum):
        print(f"Time, Bytes and Packet Number are of equal length.")
    else:
        print(f"Time, Bytes and Packet Number are not of equal length.")

    # Write the time and bytes to a CSV file    
    with open(outputPath, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        # writer.writerow(['Time', 'Bytes'])
        for t, b in zip(time, bytes):
            writer.writerow([t, b])
    print(f"Done Writing to CSV: {outputPath}")


# MAIN CODE BLOCK
if __name__ == '__main__':
    # Add your main code logic here
    # Path to the pcap file
    folder_path = '/home/drew/Thesis/LANForgeExperiments' 

    # select error rate either 5 or 10 percent
    errSel = 0 
    errRate = ['ErrRate0.05','ErrRate0.10']
    # error_rate = ["0.05","0.10"]

    start_index = 901
    end_index = 1000

    parse = 1

    folder_path = os.path.join(folder_path,errRate[errSel],"pcapFiles")

    files = [f for f in os.listdir(folder_path) if f.endswith(".pcap")]
    files.sort(key=get_run_number)
    trial = start_index
    crashed = False
    print(f"Found {len(files)} pcap files in {folder_path}.")
    print(f"File at Start Index: {files[start_index-1]} - File at End Index {files[end_index-1]}")

    if parse:
        for filename in files[start_index-1:end_index]: 
            try:
                pcapProcessor(folder_path,filename,trial)
                trial += 1
            except scapy.error.Scapy_Exception as e:
                print(f"Error processing pcap files: {e}")
        print("Done Processing pcap files")
    else:
        print("Parse Not Enabled")
pass 
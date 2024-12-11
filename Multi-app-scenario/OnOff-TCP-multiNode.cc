#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include "ns3/traffic-module.h"
#include "ns3/flow-monitor-module.h"
#include <iomanip>

#include <fstream>

using namespace ns3;
using namespace std;

string simName = "OnOff-TCP-multiNode";
NS_LOG_COMPONENT_DEFINE(simName);

/// The number of bytes to send in this simulation.
static const uint32_t totalTxBytes = 20000000; //20MB 
/// The actual number of sent bytes.
static uint32_t currentTxBytes = 0;

// Perform series of 1040 byte writes (this is a multiple of 26 since
// we want to detect data splicing in the output stream)
/// Write size.
static const uint32_t writeSize = 1040;
/// Data to be written.
uint8_t data[writeSize];

/**
 * Begin sending File from FTP Server to Client
 *
 * \param localSocket The initiating client socket Requesting File.
 * \param servAddress The server address.
 * \param servPort The server port.
 * \param fileSize The size of the file to be sent.
 */

void FTPsend(Ptr<Socket> localSocket, Ipv4Address servAddress, uint16_t servPort);

/**
 * Write to the buffer, filling it.
 *
 * \param localSocket The socket.
 * \param txSpace The number of bytes to write.
 */
void
WriteUntilBufferFull(Ptr<Socket> localSocket, uint32_t txSpace);


uint32_t timer = 10; 
uint32_t packetTotal = 0;

uint32_t totalRxDrop = 0;

/**
 * Rx drop callback
 *
 * \param p The dropped packet.
 */
static void
RxDrop(Ptr<const Packet> p)
{
    // NS_LOG_UNCOND("RxDrop at " << Simulator::Now().GetSeconds());
    totalRxDrop++; 
}

int
main(int argc, char* argv[])
{

    string runNumber = "1"; 
    string satSetup = "LEO";
    double errorRate = 0.050;

    // load in argument values from command line
    CommandLine cmd(__FILE__);
    cmd.AddValue("runNumber", "Simulation Run number => for script to iterate for pcap file management", runNumber);
    cmd.AddValue("satSetup", "Satellite Setup: GEO or LEO", satSetup);
    cmd.AddValue("errorRate", "Set Error Rate for Simulation",errorRate); 
    cmd.Parse(argc, argv);

    bool tracing = true;
    bool EMM = true;
    uint32_t Etype = 0; 
    string errType = "RateErrorModel"; 
    bool Flows = true;
    bool multipleStreams = false;
    bool variableDelay = true;


    bool tcp = false; 
    std::string prefix_file_name = "OnOff-TCP-multiNode";
    string rxPlot = "nrtvClientRxPlot";
    LogComponentEnable("OnOff-TCP-multiNode",LOG_LEVEL_INFO); 

    string typeStr = "ns3::UdpSocketFactory";
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    //string protocol = "UDP"; 

    //Adjust TCP buffer sizes if TCP is used
    if (tcp){
        tid = TypeId::LookupByName("ns3::TcpSocketFactory");
        typeStr = "ns3::TcpSocketFactory";
        //protocol = "TCP";
    }
        Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(1 << 22));
        Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1 << 22));
    NodeContainer n1n2;
    n1n2.Create(2);

    NodeContainer n2n3; //n3 is the router to the TNT network 
    n2n3.Add(n1n2.Get(1)); 
    n2n3.Create(1);

    NodeContainer nrtvAppNode; //For application 1 create endpoint node for NRTV Traffic
    nrtvAppNode.Add(n2n3.Get(1)); 
    nrtvAppNode.Create(1); 

    NodeContainer telemetryAppNode; //For application 2 create endpoint node for telemetry traffic
    telemetryAppNode.Add(n2n3.Get(1));
    telemetryAppNode.Create(1);

    NodeContainer fileDownloadUplnk; //For application 3 create endpoint node for file download traffic
    fileDownloadUplnk.Add(n2n3.Get(0));
    fileDownloadUplnk.Create(1); //create endpoint node for file download traffic

    NodeContainer fileDownloadDnlnk; 
    fileDownloadDnlnk.Add(fileDownloadUplnk.Get(0));
    fileDownloadDnlnk.Add(n1n2.Get(0));

    string uploadDelay = "165ms"; 
    string downloadDelay = "165ms";

    Ptr<UniformRandomVariable> x_up = CreateObject<UniformRandomVariable> (); 
    Ptr<UniformRandomVariable> leo_dwn = CreateObject<UniformRandomVariable> ();
    Ptr<UniformRandomVariable> leo_up = CreateObject<UniformRandomVariable> ();
    
    uint16_t variableUpDelay = 0; 
    uint16_t variableDnDelay = 0;
    string uploadRate = "20Mbps";
    string downloadRate = "40Mbps";
    
    // set the delay and data rate for the satellite link
    if (satSetup == "GEO"){
        x_up->SetAttribute("Min", DoubleValue(327));
        x_up->SetAttribute("Max", DoubleValue(338));
        variableUpDelay = x_up->GetInteger(); 
        variableDnDelay = x_up->GetInteger();
        NS_LOG_INFO("Sat Setup: " << satSetup);
    } else if (satSetup == "LEO"){
        x_up->SetAttribute("Min", DoubleValue(38));
        x_up->SetAttribute("Max", DoubleValue(55));
        // leo_dwn->SetAttribute("Min", DoubleValue(40));
        // leo_dwn->SetAttribute("Max", DoubleValue(225));
        // leo_up->SetAttribute("Min", DoubleValue(10));
        // leo_up->SetAttribute("Max", DoubleValue(25));
        variableUpDelay = x_up->GetInteger(); 
        variableDnDelay = x_up->GetInteger();
        uploadRate = "25Mbps"; //to_string(leo_up->GetInteger())+"Mbps";
        downloadRate = "225Mbps"; //to_string(leo_dwn->GetInteger())+"Mbps";
        NS_LOG_INFO("Sat Setup: " << satSetup);
        //print upload and download rates
        NS_LOG_INFO("Upload Rate: " << uploadRate);
        NS_LOG_INFO("Download Rate: " << downloadRate);

    } else {
        variableDelay = false;
    }
    

    if (variableDelay) {
        uploadDelay = to_string(variableUpDelay)+"ms";
        downloadDelay = to_string(variableDnDelay)+"ms";
    }
    

    // Set up the point-to-point links
    PointToPointHelper p2pUpload;
    p2pUpload.SetDeviceAttribute("DataRate", StringValue(uploadRate));
    p2pUpload.SetChannelAttribute("Delay", StringValue(uploadDelay));
    // p2pUpload.SetQueue("ns3::DropTailQueue", "MaxPackets", UintegerValue(0xffffffff));

    // Set up the point-to-point links
    PointToPointHelper p2pDownload;
    p2pDownload.SetDeviceAttribute("DataRate", StringValue(downloadRate));
    p2pDownload.SetChannelAttribute("Delay", StringValue(downloadDelay));

    PointToPointHelper TNTp2p; 
    TNTp2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    TNTp2p.SetChannelAttribute("Delay", StringValue("0ms"));

    // Install the devices
    NetDeviceContainer n1n2devices;
    n1n2devices = p2pUpload.Install(n1n2);

    NetDeviceContainer n2n3devices;
    n2n3devices = p2pDownload.Install(n2n3);

    NetDeviceContainer app1Devices;
    app1Devices = TNTp2p.Install(nrtvAppNode);//n2n3.Get(1)) and nrtvAppNode.Get(1)

    NetDeviceContainer app2Devices;
    app2Devices = TNTp2p.Install(telemetryAppNode); //n2n3.Get(1) and telemetryAppNode.Get(1)

    NetDeviceContainer app3DevsUp;
    app3DevsUp = p2pUpload.Install(fileDownloadUplnk); //n2n3.Get(1) and fileDownloadUplnk.Get(1)

    NetDeviceContainer app3DevsDn;
    app3DevsDn = p2pDownload.Install(fileDownloadDnlnk); //fileDownloadUplnk.Get(1) and n1n2.Get(0)
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3) << errorRate;

    string ErrorString = oss.str();
    if (EMM){ 
        std::string Error = std::to_string(errorRate);
        // Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
        // em->SetAttribute("ErrorRate", DoubleValue(errorRate));
        // devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    }

    // Set a few attributes
    Config::SetDefault("ns3::RateErrorModel::ErrorRate", DoubleValue(errorRate));
    Config::SetDefault("ns3::RateErrorModel::ErrorUnit", StringValue("ERROR_UNIT_PACKET"));

    Config::SetDefault("ns3::BurstErrorModel::ErrorRate", DoubleValue(0.01));
    Config::SetDefault("ns3::BurstErrorModel::BurstSize",
                       StringValue("ns3::UniformRandomVariable[Min=1|Max=3]"));

    std::string errorModelType = "ns3::RateErrorModel";
    ObjectFactory factory;
    // factory.SetTypeId(errorModelType);
    // Ptr<ErrorModel> em = factory.Create<ErrorModel>();
    Ptr<RateErrorModel> em;
    if (Etype == 0){
        NS_LOG_INFO("Error Model Type: Rate Error Model"); 
        em = CreateObject<RateErrorModel>();

        em->SetAttribute("ErrorRate", DoubleValue(errorRate)); 
        n1n2devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));
        app3DevsUp.Get(0)->SetAttribute("ReceiveErrorModel",PointerValue(em)); 
        n2n3devices.Get(0)->SetAttribute("ReceiveErrorModel",PointerValue(em));
    } else if(Etype == 1){ 
        errorModelType = "ns3::BurstErrorModel";
        NS_LOG_INFO("Error Model Type: Burst Error Model");
        errType = "BurstErrorModel";

        Ptr<BurstErrorModel> emB = CreateObject<BurstErrorModel>();
        em->SetAttribute("ErrorRate", DoubleValue(errorRate)); 
        n1n2devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    } else if (Etype ==2) {
        NS_LOG_INFO("Other Method used");
        // std::string Error = std::to_string(errorRate);
        
        Ptr<RateErrorModel> em2 = CreateObject<RateErrorModel>();
        em2->SetAttribute("ErrorRate", DoubleValue(errorRate));
        n1n2devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em2));
    }
    
    // Install the internet stack on all nodes
    InternetStackHelper stack;
    stack.Install(n1n2);
    stack.Install(n2n3.Get(1));
    stack.Install(nrtvAppNode.Get(1));
    stack.Install(telemetryAppNode.Get(1));
    stack.Install(fileDownloadUplnk.Get(1));

    // Assign IP addresses
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.252");
    Ipv4InterfaceContainer interfaces = address.Assign(n1n2devices);

    address.SetBase("10.2.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces2 = address.Assign(n2n3devices);

    address.SetBase("10.3.1.0", "255.255.255.0");
    Ipv4InterfaceContainer app1Interfaces = address.Assign(app1Devices);

    address.SetBase("10.4.1.0", "255.255.255.0");
    Ipv4InterfaceContainer app2Interfaces = address.Assign(app2Devices);

    address.SetBase("10.5.1.0", "255.255.255.0");
    Ipv4InterfaceContainer app3Interfaces = address.Assign(app3DevsUp);
    
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    double appStoptime = 205.0;
    double simStomtime = appStoptime + .25;
    Time startTime = MilliSeconds(1.0);
    Time AppStopTime = Seconds(appStoptime);
    Time simStopTime = Seconds(simStomtime);

    /**********************************
    Application Number 1: NRTV Traffic
    ***********************************/

    NrtvHelper nrtvHelper(tid);
    nrtvHelper.InstallUsingIpv4(n1n2.Get(0), nrtvAppNode.Get(1)); 

    if (multipleStreams){
        nrtvHelper.SetVariablesAttribute("NumberOfVideos",
                                     StringValue("ns3::UniformRandomVariable[Min=3|Max=3]"));    
    }

    ApplicationContainer sourceApps = nrtvHelper.GetServer();
    sourceApps.Start(startTime);
    sourceApps.Stop(AppStopTime);

    /**********************************
    Application Number 2: Telemetry Data
    ***********************************/ 
    uint16_t servPort = 9;
    uint32_t packetSize = 5000;
    string dataRate = "40Mb/s";
    OnOffHelper onoff("ns3::TcpSocketFactory",
                      Address(InetSocketAddress(app2Interfaces.GetAddress(1), servPort))); //remote address
    // onoff.SetConstantRate(DataRate("10Mb/s"));
    onoff.SetAttribute("DataRate", StringValue(dataRate));
    onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=10]"));
    onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onoff.SetAttribute("PacketSize",UintegerValue(packetSize)); 
    ApplicationContainer telemetryApp = onoff.Install(n1n2.Get(0));
    telemetryApp.Start(startTime);
    telemetryApp.Stop(AppStopTime);

    // Create an optional packet sink to receive these packets
    PacketSinkHelper telSink("ns3::TcpSocketFactory",
                          Address(InetSocketAddress(Ipv4Address::GetAny(), servPort)));
    telemetryApp = telSink.Install(telemetryAppNode.Get(1));
    // telemetryApp.Start(startTime);
    // telemetryApp.Stop(AppStopTime);

    /**********************************
    Application Number 3: HTTP File Download
    ***********************************/

   PacketSinkHelper sink("ns3::TcpSocketFactory",
                          InetSocketAddress(Ipv4Address::GetAny(), servPort));

    ApplicationContainer apps = sink.Install(n1n2.Get(0));
    apps.Start(startTime);
    apps.Stop(AppStopTime);

   // Create and bind the socket...
    Ptr<Socket> localSocket = Socket::CreateSocket(fileDownloadUplnk.Get(1), TcpSocketFactory::GetTypeId());
    localSocket->Bind();
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(1 << 23)); //8MB
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1 << 23));

    localSocket->SetAttribute("SndBufSize", UintegerValue(1<<22));//1MB
    // localSocket->SetAttribute("SndBufSize", UintegerValue(4096));//1MB

    // not used but note the syntax for setting the congestion window
    // Trace changes to the congestion window
    // Config::ConnectWithoutContext("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow",
    //                               MakeCallback(&CwndTracer));

    //  Address clientAddress = interfaces.GetAddress(0);

    // Schedule the FTP send to start at time zero
    Simulator::ScheduleNow(&FTPsend, localSocket, interfaces.GetAddress(0), servPort);
    
    ApplicationContainer sinkApps;
    sinkApps = nrtvHelper.GetClients(); //sinkApps;
    sinkApps.Start(startTime);
    sinkApps.Stop(AppStopTime); 

    //get node id of n1n2.Get(1)
    uint16_t app1NodeId = nrtvAppNode.Get(1)->GetId();
    uint16_t app2NodeID = telemetryAppNode.Get(1)->GetId();
    uint16_t app3NodeID = fileDownloadUplnk.Get(1)->GetId();

    //get device ID of n1n2.Get(1)
    uint16_t app1DeviceID = app1Devices.Get(1)->GetIfIndex();
    uint16_t app2DeviceID = app2Devices.Get(1)->GetIfIndex();
    uint16_t app3DeviceID = app3DevsUp.Get(1)->GetIfIndex();

    
     //APP1-combined-err-0.001-Run_1.pcap


    // Enable Tracing
    if (tracing){

        string dataFolderApp1 = "/home/drew/Thesis/pcapData/ErrRate"+ErrorString+"/NRTV-app1/APP1-combined-multi-node_Run_"+runNumber;
        string dataFolderApp2 = "/home/drew/Thesis/pcapData/ErrRate"+ErrorString+"/telemetryData-app2/APP2-combined-multi-node_Run_"+runNumber;
        string datafolderApp3 = "/home/drew/Thesis/pcapData/ErrRate"+ErrorString+"/fileTransfer-app3/APP3-combined-multi-node_Run_"+runNumber;
        if (satSetup == "LEO"){
            dataFolderApp1 = "/home/drew/Thesis/pcapData/LEO/ErrRate"+ErrorString+"/NRTV-app1/APP1-combined-multi-node_Run_"+runNumber;
            dataFolderApp2 = "/home/drew/Thesis/pcapData/LEO/ErrRate"+ErrorString+"/telemetryData-app2/APP2-combined-multi-node_Run_"+runNumber;
            datafolderApp3 = "/home/drew/Thesis/pcapData/LEO/ErrRate"+ErrorString+"/fileTransfer-app3/APP3-combined-multi-node_Run_"+runNumber;
        }

        // p2pDownload.EnablePcapAll(simName);
        p2pUpload.EnablePcap(dataFolderApp1, app1NodeId, app1DeviceID, false);
        p2pUpload.EnablePcap(dataFolderApp2, app2NodeID, app2DeviceID, false);
        p2pUpload.EnablePcap(datafolderApp3, app3NodeID, app3DeviceID, false);
        n1n2devices.Get(1)->TraceConnectWithoutContext("PhyRxDrop", MakeCallback(&RxDrop));
        
    }

    NS_LOG_INFO("Starting Run Number: " << runNumber);
    if (EMM)
    {
    NS_LOG_INFO("Error Model Enabled");
    // NS_LOG_INFO("Error Model Type:" << errType);
    NS_LOG_INFO("Error Amount: " << errorRate);
    } else {
        NS_LOG_INFO("No Error Model Enabled");
    }

    // Simulator::Stop(stopTime);
    // Simulator::Run();
    
////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //flow monitor for output and tracking
    if (Flows) {

        FlowMonitorHelper flowmonHelper;
        NodeContainer endpointNodes;
        endpointNodes.Add(n1n2.Get(0));
        endpointNodes.Add(n1n2.Get(1));
        // endpointNodes.Add(n2n3.Get(1));
        endpointNodes.Add(nrtvAppNode.Get(1));
        endpointNodes.Add(telemetryAppNode.Get(1));
        endpointNodes.Add(fileDownloadUplnk.Get(1));

        Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install(endpointNodes);
        monitor->SetAttribute("DelayBinWidth", DoubleValue(0.001));
        monitor->SetAttribute("JitterBinWidth", DoubleValue(0.001));
        monitor->SetAttribute("PacketSizeBinWidth", DoubleValue(20));



        Simulator::Stop(simStopTime);
        Simulator::Run();
        Simulator::Destroy();

        //Print out many bytes receieved on sink node
        Ptr<PacketSink> sink1 = DynamicCast<PacketSink>(sinkApps.Get(0));
        uint32_t totalRx = sink1->GetTotalRx(); 
        std::cout << "\nNRTV APP Total Bytes Received: " << totalRx << std::endl;
        std::cout << "NRTV APP Calculated Throughput: " << totalRx * 8 / appStoptime / 1000 << " kbps\n" << std::endl;

        
       
        //print out bytes received on telemetry app
        Ptr<PacketSink> sink2 = DynamicCast<PacketSink>(telemetryApp.Get(0));
        uint32_t totalRxTeleApp = sink2->GetTotalRx();
        std::cout << "\nTelemetry APP Total Bytes Received: " << totalRxTeleApp << std::endl;
        std::cout << "Telemetry APP Calculated Throughput: " << totalRxTeleApp * 8 / appStoptime / 1000 << " kbps\n" << std::endl;

        //print out bytes received on file download app
        Ptr<PacketSink> sink3 = DynamicCast<PacketSink>(apps.Get(0));
        uint32_t totalClientRx = sink3->GetTotalRx();
        std::cout << "\nFile Download APP Total Bytes Received: " << totalClientRx << std::endl;
        std::cout << "File Download APP Calculated Throughput: " << totalClientRx * 8 / appStoptime / 1000 << " kbps\n" << std::endl;
        
        if (variableDelay){
        NS_LOG_INFO("Variable Delay Used:\n");
        }
        cout << "Set Upload Data Rate: " << uploadRate << endl;
        cout << "Set Download Data Rate: " << downloadRate << endl; 
        cout << "Set Ulink Delay: " << uploadDelay << endl; 
        cout << "Set Dlink Delay: " << downloadDelay << endl;
        std::cout << "Total RX Drops: " << totalRxDrop << std::endl; 
        // std::cout << "Packet Loss Ratio: " << totalRxDrop * 100.0 / 8000 << " %" << std::endl;

        std::cout << "App Total Time: " << to_string(appStoptime) << "Seconds" << std::endl;
        std::cout << "Simulation Total Time: " << to_string(simStomtime) << "Seconds\n\n" << std::endl;
            

        // Print per-flow statistics
        monitor->CheckForLostPackets();
        Ptr<Ipv4FlowClassifier> classifier =
            DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier());
        FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

        double averageFlowThroughput = 0.0;
        double averageFlowDelay = 0.0;

        string outputDir = "/home/drew/Thesis/satStatistics/OnOff-TCP-multiNode";
        string simTag = "Run_"+runNumber+simName+std::to_string(errorRate);//+"-"+errType+"-"+std::to_string(errorRate)+"-"+uploadRate+"-"+delay+".txt";
        std::ofstream outFile;
        std::string filename = outputDir + "/" + simTag;
        outFile.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
        if (!outFile.is_open())
        {
            std::cerr << "Can't open file " << filename << std::endl;
            return 1;
        }

        outFile.setf(std::ios_base::fixed);

        double flowDuration = (AppStopTime - startTime).GetSeconds();
        for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin();
            i != stats.end();
            ++i)
        {
            Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
            std::stringstream protoStream;
            protoStream << (uint16_t)t.protocol;
            if (t.protocol == 6)
            {
                protoStream.str("TCP");
            }
            if (t.protocol == 17)
            {
                protoStream.str("UDP");
            }
            outFile << "Flow " << i->first << " (" << t.sourceAddress << ":" << t.sourcePort << " -> "
                    << t.destinationAddress << ":" << t.destinationPort << ") proto "
                    << protoStream.str() << "\n";
            outFile << "  Tx Packets: " << i->second.txPackets << "\n";
            outFile << "  Tx Bytes:   " << i->second.txBytes << "\n";
            outFile << "  TxOffered:  " << i->second.txBytes * 8.0 / flowDuration / 1000.0 << " kbps\n";
            outFile << "  Rx Bytes:   " << i->second.rxBytes << "\n";
            double bytesPerPacketTX = i->second.txBytes / i->second.txPackets;
            double bytesPerPacketRX = i->second.rxBytes / i->second.rxPackets;
            outFile << "  TX Bytes Per Packet: " << bytesPerPacketTX << "\n";
            outFile << "  RX Bytes Per Packet: " << bytesPerPacketRX << "\n";

            if (i->second.rxPackets > 0)
            {
                // Measure the duration of the flow from receiver's perspective
                averageFlowThroughput += i->second.rxBytes * 8.0 / flowDuration / 1000 / 1000;
                averageFlowDelay += 1000 * i->second.delaySum.GetSeconds() / i->second.rxPackets;

                outFile << "  Throughput: " << i->second.rxBytes * 8.0 / flowDuration / 1000 << " kbps\n";
                outFile << "  Mean delay:  "
                        << 1000 * i->second.delaySum.GetSeconds() / i->second.rxPackets << " ms\n";
                // outFile << "  Mean upt:  " << i->second.uptSum / i->second.rxPackets / 1000/1000 << "
                // Mbps \n";
                outFile << "  Mean jitter:  "
                        << 1000 * i->second.jitterSum.GetSeconds() / i->second.rxPackets << " ms\n";
            }
            else
            {
                outFile << "  Throughput:  0 Mbps\n";
                outFile << "  Mean delay:  0 ms\n";
                outFile << "  Mean jitter: 0 ms\n";
            }
            outFile << "  Rx Packets: " << i->second.rxPackets << "\n";
            outFile << "  Lost Packets: " << i->second.lostPackets << "\n";
            outFile << "  Packet loss ratio: " << i->second.lostPackets * 100.0 / i->second.txPackets
                    << " %\n";
        }

        outFile << "\n\n  Mean flow throughput: " << averageFlowThroughput / stats.size()<< " Mbps" << "\n";
        outFile << "  Mean flow throughput: " << averageFlowThroughput*1000 / stats.size()<< " kbps" << "\n";
        outFile << "  Mean flow delay: " << averageFlowDelay / stats.size()<< "ms" << "\n";

        outFile.close();

        std::ifstream f(filename.c_str());

        if (f.is_open())
        {
            std::cout << f.rdbuf();
        }

        monitor->SerializeToXmlFile("/home/drew/Thesis/satStatistics/OnOff-TCP-multiNode/flowMonOutput-"+std::to_string(errorRate)+""+".xml", true, true);

    }

    // Simulator::Destroy();
    return 0;

}

// Begin sending File from FTP Server to Client
void FTPsend(Ptr<Socket> localSocket, Ipv4Address servAddress, uint16_t servPort) 
{
    NS_LOG_LOGIC("Starting flow at time " << Simulator::Now().GetSeconds());
    localSocket->Connect(InetSocketAddress(servAddress, servPort)); // connect

    localSocket->SetSendCallback(MakeCallback(&WriteUntilBufferFull));
    WriteUntilBufferFull(localSocket, localSocket->GetTxAvailable());


}


// Write to the buffer, filling it.
void
WriteUntilBufferFull(Ptr<Socket> localSocket, uint32_t txSpace)
{
    while (currentTxBytes < totalTxBytes && localSocket->GetTxAvailable() > 0)
    {
        uint32_t left = totalTxBytes - currentTxBytes;
        uint32_t dataOffset = currentTxBytes % writeSize;
        uint32_t toWrite = writeSize - dataOffset;
        toWrite = std::min(toWrite, left);
        toWrite = std::min(toWrite, localSocket->GetTxAvailable());
        int amountSent = localSocket->Send(&::data[dataOffset], toWrite, 0);
        if (amountSent < 0)
        {
            // we will be called again when new tx space becomes available.
            return;
        }
        currentTxBytes += amountSent;
    }
    if (currentTxBytes >= totalTxBytes)
    {
        localSocket->Close();
        // NS_LOG_INFO("CurrentTXBytes at Close: "<< currentTxBytes); 

    }
}

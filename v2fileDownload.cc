#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"


#include "ns3/traffic-module.h"
#include "ns3/flow-monitor-module.h"
#include <iomanip>


#include <fstream>
#include <iostream>
#include <string>

using namespace ns3;
using namespace std;

std::string simName = "v2fileDownloadApp";
NS_LOG_COMPONENT_DEFINE(simName);

/// The number of bytes to send in this simulation.
static const uint32_t totalTxBytes = 20000000;
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

// void FTPsend(Ptr<Socket> clientSocket, Ipv4Address servAddress, uint16_t servPort, uint32_t fileSize) 
// {
//     NS_LOG_LOGIC("Starting flow at time " << Simulator::Now().GetSeconds());
//     clientSocket->Connect(InetSocketAddress(servAddress, servPort)); // connect

//     Ptr<Packet> packet = Create<Packet>(fileSize); // Simulate sending file
//     clientSocket->Send(packet);

//     NS_LOG_INFO("Sent file to client"); 
// }

/**
 * Write to the buffer, filling it.
 *
 * \param localSocket The socket.
 * \param txSpace The number of bytes to write.
 */
void
WriteUntilBufferFull(Ptr<Socket> localSocket, uint32_t txSpace);

// uint32_t totalRxDrop = 0;

// /**
//  * Rx drop callback
//  *
//  * \param p The dropped packet.
//  */
// static void
// RxDrop(Ptr<const Packet> p)
// {
//     // NS_LOG_UNCOND("RxDrop at " << Simulator::Now().GetSeconds());
//     totalRxDrop++; 
// }

/**
 * Generates traffic.
 *
 * The first call sends a packet of the specified size, and then
 * the function is scheduled to send a packet of (size-50) after 0.5s.
 * The process is iterated until the packet size is zero.
 *
 * \param socket output socket
 * \param size packet size
 */
// static void
// GenerateTraffic(Ptr<Socket> socket, int32_t size)
// {
//     if (size <= 0)
//     {
//         socket->Close();
//         return;
//     }

//     std::cout << "at=" << Simulator::Now().GetSeconds() << "s, tx bytes=" << size << std::endl;
//     socket->Send(Create<Packet>(size));
//     Simulator::Schedule(Seconds(0.5), &GenerateTraffic, socket, size - 50);
// }

// /**
//  * Prints the packets received by a socket
//  * \param socket input socket
//  */
// static void
// SocketPrinter(Ptr<Socket> socket)
// {
//     Ptr<Packet> packet;
//     while ((packet = socket->Recv()))
//     {
//         std::cout << "at=" << Simulator::Now().GetSeconds() << "s, rx bytes=" << packet->GetSize()
//                   << std::endl;
//     }
// }

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
    string runNumber = "1000"; 
    string satSetup = "LEO";
    double errorRate = 0.001;
    CommandLine cmd(__FILE__);
    cmd.AddValue("runNumber", "Simulation Run number => for script to iterate for pcap file management", runNumber);
    cmd.AddValue("satSetup", "Satellite Setup: GEO or LEO", satSetup);
    cmd.AddValue("errorRate", "Set Error Rate for Simulation",errorRate);
    cmd.Parse(argc, argv);

    // bool tracing = true;
    bool EMM = true;
    uint32_t Etype = 0; //SET TO 0 -- 0 = RateErrorModel, 1 = BurstErrorModel, 2 = Other
    string errType = "RateErrorModel"; 
    bool Flows = true;
    bool TNT = true;
    bool rtnDrop = true;
    bool variableDelay = true;

    // initialize the tx buffer.
    for (uint32_t i = 0; i < writeSize; ++i)
    {
        char m = toascii(97 + i % 26);
        ::data[i] = m;
    }

    std::string prefix_file_name = simName;
    string rxPlot = "fileDownloadClientRxPlot";
    LogComponentEnable("v2fileDownloadApp",LOG_LEVEL_INFO); 
    // LogComponentEnable("TcpSocketBase", LOG_LEVEL_INFO);

    string protocol = "TCP"; 
    // bool rttTrace = 0;
    
    TypeId tid = TypeId::LookupByName("ns3::TcpSocketFactory");
    std::string typeStr = "ns3::TcpSocketFactory";
    protocol = "TCP";
    // Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(1 << 22));
    // Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1 << 22));

    NodeContainer n1n2;
    n1n2.Create(2);

    NodeContainer n2n3; 
    n2n3.Add(n1n2.Get(1)); 
    n2n3.Create(1);

    string uploadDelay = "165ms"; 
    string downloadDelay = "165ms";

    Ptr<UniformRandomVariable> x_up = CreateObject<UniformRandomVariable> (); 
    Ptr<UniformRandomVariable> leo_dwn = CreateObject<UniformRandomVariable> ();
    Ptr<UniformRandomVariable> leo_up = CreateObject<UniformRandomVariable> ();

    
    uint16_t variableUpDelay = 0; 
    uint16_t variableDnDelay = 0;
    string uploadRate = "20Mbps";
    string downloadRate = "40Mbps";

    if (satSetup == "GEO"){
        x_up->SetAttribute("Min", DoubleValue(327));
        x_up->SetAttribute("Max", DoubleValue(338));
        variableUpDelay = x_up->GetInteger(); 
        variableDnDelay = x_up->GetInteger();
    } else if (satSetup == "LEO"){
        x_up->SetAttribute("Min", DoubleValue(38));
        x_up->SetAttribute("Max", DoubleValue(55));
        leo_dwn->SetAttribute("Min", DoubleValue(40));
        leo_dwn->SetAttribute("Max", DoubleValue(225));
        leo_up->SetAttribute("Min", DoubleValue(10));
        leo_up->SetAttribute("Max", DoubleValue(25));
        variableUpDelay = x_up->GetInteger(); 
        variableDnDelay = x_up->GetInteger();
        uploadRate = to_string(leo_up->GetInteger())+"Mbps";
        downloadRate = to_string(leo_dwn->GetInteger())+"Mbps";
    } else {
        variableDelay = false;
    }
    

    if (variableDelay) {
        uploadDelay = to_string(variableUpDelay)+"ms";
        downloadDelay = to_string(variableDnDelay)+"ms";
    }
    
    PointToPointHelper p2pUpload;
    p2pUpload.SetDeviceAttribute("DataRate", StringValue(uploadRate));
    p2pUpload.SetChannelAttribute("Delay", StringValue(uploadDelay));
    // p2pUpload.SetQueue("ns3::DropTailQueue", "MaxPackets", UintegerValue(0xffffffff));

    PointToPointHelper p2pDownload;
    p2pDownload.SetDeviceAttribute("DataRate", StringValue(downloadRate));
    p2pDownload.SetChannelAttribute("Delay", StringValue(downloadDelay));

    NetDeviceContainer n1n2devices;
    n1n2devices = p2pUpload.Install(n1n2);

    NetDeviceContainer n2n3devices;
    n2n3devices = p2pDownload.Install(n2n3);

    

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

    // Config::SetDefault("ns3::BurstErrorModel::ErrorRate", DoubleValue(0.01));
    // Config::SetDefault("ns3::BurstErrorModel::BurstSize",
    //                    StringValue("ns3::UniformRandomVariable[Min=1|Max=3]"));

    std::string errorModelType = "ns3::RateErrorModel";
    ObjectFactory factory;
    // factory.SetTypeId(errorModelType);
    // Ptr<ErrorModel> em = factory.Create<ErrorModel>();
    string rtnDropEnable = "Disabled";
    Ptr<RateErrorModel> em;
    if (Etype == 0){
        NS_LOG_INFO("Error Model Type: Rate Error Model"); 
        em = CreateObject<RateErrorModel>();

        em->SetAttribute("ErrorRate", DoubleValue(errorRate)); 
        n1n2devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));
        if (rtnDrop){
            n2n3devices.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(em));
            rtnDropEnable = "Enabled";
        }
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
    InternetStackHelper stack;
    stack.Install(n1n2);
    stack.Install(n2n3.Get(1));

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.252");
    Ipv4InterfaceContainer interfaces = address.Assign(n1n2devices);

    address.SetBase("10.2.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces2 = address.Assign(n2n3devices);

    NodeContainer n3nTnT; 
    n3nTnT.Add(n2n3.Get(1));
    n3nTnT.Create(1);

    PointToPointHelper TNTp2p;
    TNTp2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    TNTp2p.SetChannelAttribute("Delay", StringValue("0ms"));

    NetDeviceContainer TNTdevices;
    TNTdevices = TNTp2p.Install(n3nTnT);

    stack.Install(n3nTnT.Get(1));

    address.SetBase("10.3.1.0", "255.255.255.0");
    Ipv4InterfaceContainer TNTinterfaces = address.Assign(TNTdevices);
    
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // NodeContainer TnTn2; //TNT node to Satellite Node 
    // TnTn2.Add(n3nTnT.Get(1));
    // TnTn2.Add(n2n3.Get(0));

    // NetDeviceContainer returnChannel1 = p2pUpload.Install(TnTn2);

    // NetDeviceContainer returnChannel2 = p2pDownload.Install(TnTn2.Get(1),n1n2.Get(0));

    // address.SetBase("10.3.2.0", "255.255.255.0");
    // Ipv4InterfaceContainer returnInterfaces1 = address.Assign(returnChannel1);

    ApplicationContainer sourceApps; 
   
    double appStoptime = 205.0;
    double simStomtime = appStoptime + .25;
    Time startTime = MilliSeconds(1.0);
    Time AppStopTime = Seconds(appStoptime);
    Time simStopTime = Seconds(simStomtime);


    Ipv4Address remoteAddress = interfaces2.GetAddress(1);

    Ptr<Node> destNode = n2n3.Get(1);
    if (TNT) {
        remoteAddress = TNTinterfaces.GetAddress(1);
        destNode = n3nTnT.Get(1);
    }

    uint16_t servPort = 49200;
    /**********************************
    Application Number 3: HTTP File Download
    ***********************************/
    // Create a packet sink to receive these packets on n2...
    PacketSinkHelper sink("ns3::TcpSocketFactory",
                          InetSocketAddress(Ipv4Address::GetAny(), servPort));

    ApplicationContainer apps = sink.Install(n1n2.Get(0));
    apps.Start(startTime);
    apps.Stop(AppStopTime);

   // Create and bind the socket...
    Ptr<Socket> localSocket = Socket::CreateSocket(n3nTnT.Get(1), TcpSocketFactory::GetTypeId());
    localSocket->Bind();
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(1 << 23)); //8MB
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1 << 23));

    localSocket->SetAttribute("SndBufSize", UintegerValue(1<<22));//1MB
    // localSocket->SetAttribute("SndBufSize", UintegerValue(4096));

    // Trace changes to the congestion window
    // Config::ConnectWithoutContext("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow",
    //                               MakeCallback(&CwndTracer));

    // uint32_t fileSize = 8192; //1,048,576 bytes -> 1MB

    Simulator::ScheduleNow(&FTPsend, localSocket, interfaces.GetAddress(0), servPort);

    std::string tntEnable = "Disabled";
    if (TNT) {
        tntEnable = "Enabled";
    }

    // auto apps = sinkApps; //sinkApps;
    // apps.Start(startTime);
    // uint32_t i = 0;
    // std::vector<Ptr<ClientRxTracePlot>> plots;
    // for (auto app = apps.Begin(); app != apps.End(); app++, i++)
    // {
    //     std::stringstream fileDownloadHTTP;
    //     fileDownloadHTTP << "TCP" <<"-FileDownload-" << "ClientRxPlot"<<"TNT-"<<tntEnable <<"RtnDrop-"<<rtnDropEnable;
    //     plots.push_back(CreateObject<ClientRxTracePlot>(*app, fileDownloadHTTP.str()));
    // }

    NS_LOG_INFO("Starting v2FileDownload...");
    NS_LOG_INFO("Satellite Setup: " << satSetup);
    NS_LOG_INFO("L4 Protocol: " << protocol);
    if (EMM)
    {
    NS_LOG_INFO("Error Model Enabled");
    NS_LOG_INFO("TNT: " << tntEnable);  
    NS_LOG_INFO("Return Link Drop: " << rtnDropEnable);
    // NS_LOG_INFO("Error Model Type:" << errType);
    NS_LOG_INFO("Error Amount: " << errorRate);
    } else {
        NS_LOG_INFO("No Error Model Enabled");
    }

    uint16_t nodeID = n3nTnT.Get(1)->GetId();

    uint16_t deviceID = TNTdevices.Get(1)->GetIfIndex();

////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // p2pDownload.EnablePcapAll("v2FileDownload");
    // p2pUpload.EnablePcapAll("v2FileDownload");

    string outputDir = "/home/drew/Thesis/satStatistics/v2fileDownload-test";
    string simTag = "v2fileDownload-baseline";

    string dataFolder = "/home/drew/Thesis/baselinePcapData/ErrRate"+ErrorString+"/fileTransfer-app3/"+simTag+"_Run_"+runNumber;
        if (satSetup == "LEO"){
            dataFolder = "/home/drew/Thesis/baselinePcapData/LEO/ErrRate"+ErrorString+"/fileTransfer-app3/"+simTag+"_Run_"+runNumber;
        }

    p2pUpload.EnablePcap(dataFolder, nodeID,deviceID, false);
    n1n2devices.Get(1)->TraceConnectWithoutContext("PhyRxDrop", MakeCallback(&RxDrop));


    if (Flows) {

        FlowMonitorHelper flowmonHelper;
        NodeContainer endpointNodes;
        endpointNodes.Add(n1n2.Get(0));
        // endpointNodes.Add(n1n2.Get(1));
        endpointNodes.Add(n2n3.Get(1));
        endpointNodes.Add(n3nTnT.Get(1));

        Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install(endpointNodes);
        monitor->SetAttribute("DelayBinWidth", DoubleValue(0.001));
        monitor->SetAttribute("JitterBinWidth", DoubleValue(0.00001));
        monitor->SetAttribute("PacketSizeBinWidth", DoubleValue(20));

        Simulator::Stop(simStopTime);
        Simulator::Run();
        Simulator::Destroy();

        //Print out many bytes receieved on sink node

        // Ptr<PacketSink> sink1 = DynamicCast<PacketSink>(sinkApps.Get(0));
        // uint32_t totalRx = sink1->GetTotalRx(); 
        // std::cout << "\nTotal Bytes Received: " << totalRx << std::endl;
        // std::cout << "Calculated Throughput: " << totalRx * 8 / appStoptime / 1000 << " kbps\n" << std::endl;
        //print out bytes received on file download app
        // std::cout << "\nFile Download APP Total Bytes Received: " << totalClientRx << std::endl;
        // std::cout << "File Download APP Calculated Throughput: " << totalClientRx * 8 / appStoptime / 1000 << " kbps\n" << std::endl;

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

        // string outputDir = "/home/drew/Thesis/satStatistics/v2fileDownload-test";
        // string simTag = "v2fileDownload-baseline";//+"-"+errType+"-"+std::to_string(errorRate)+"-"+uploadRate+"-"+delay+".txt";
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

        monitor->SerializeToXmlFile(
            "/home/drew/Thesis/satStatistics/v2fileDownload-test/V2FileDownloadflowMonOutput-"+std::to_string(errorRate)+".xml", 
            true, true);

    }

    // Simulator::Destroy();
    return 0;



}

void FTPsend(Ptr<Socket> localSocket, Ipv4Address servAddress, uint16_t servPort) 
{
    NS_LOG_LOGIC("Starting flow at time " << Simulator::Now().GetSeconds());
    localSocket->Connect(InetSocketAddress(servAddress, servPort)); // connect

    localSocket->SetSendCallback(MakeCallback(&WriteUntilBufferFull));
    WriteUntilBufferFull(localSocket, localSocket->GetTxAvailable());


}

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
    }
}
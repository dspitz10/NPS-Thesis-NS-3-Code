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

NS_LOG_COMPONENT_DEFINE("three-node-tcp-OnOff");

uint32_t timer = 10; 
uint32_t packetTotal = 0;

static uint32_t
GetNodeIdFromContext(std::string context)
{
    std::size_t const n1 = context.find_first_of('/', 1);
    std::size_t const n2 = context.find_first_of('/', n1 + 1);
    return std::stoul(context.substr(n1 + 1, n2 - n1 - 1));
}

void
ReceivePacket(Ptr<Socket> socket)
{
    if (packetTotal < 10){
        NS_LOG_INFO("Received one packet!");
    }
    // Ptr<Packet> packet = socket->Recv();
    packetTotal++; 
}

std::ofstream RTTtrace("tcp-NRTV-rtt-trace-file.txt");
static std::map<uint32_t, bool> firstRtt; //!< First RTT
static std::map<uint32_t, Ptr<OutputStreamWrapper>> rttStream; //!< RTT outut stream.

/**
 * RTT tracer.
 *
 * \param context The context.
 * \param oldval Old value.
 * \param newval New value.
 */
static void
RttTracer(std::string context, Time oldval, Time newval)
{
    uint32_t nodeId = GetNodeIdFromContext(context);

    if (firstRtt[nodeId])
    {
        RTTtrace << "0.0 " << oldval.GetSeconds() << std::endl;
        // *rttStream[nodeId]->GetStream() << "0.0 " << oldval.GetSeconds() << std::endl;
        firstRtt[nodeId] = false;
    }
    RTTtrace << Simulator::Now().GetSeconds() << "" <<newval.GetSeconds() << std::endl;
    // *rttStream[nodeId]->GetStream()
        // << Simulator::Now().GetSeconds() << " " << newval.GetSeconds() << std::endl;
}


/**
 * RTT trace connection.
 *
 * \param rtt_tr_file_name RTT trace file name.
 * \param nodeId Node ID.
 */
static void
TraceRtt(std::string rtt_tr_file_name, uint32_t nodeId)
{
    NS_LOG_INFO("Node Id: " << nodeId);
    AsciiTraceHelper ascii;
    rttStream[nodeId] = ascii.CreateFileStream(rtt_tr_file_name);
    Config::Connect("/NodeList/" + std::to_string(nodeId) + "/$ns3::TcpL4Protocol/SocketList/0/RTT",
                    MakeCallback(&RttTracer));
}

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
    string runNumber = "2"; 
    string satSetup = "GEO";
    double errorRate = 0.001;

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
    bool TNT = true;
    bool rtnDrop = true;
    bool variableDelay = true;

    // Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpCubic"));


    bool tcp = false; 
    std::string prefix_file_name = "three-node-tcp-OnOff";
    string rxPlot = "nrtvClientRxPlot";
    LogComponentEnable("three-node-tcp-OnOff",LOG_LEVEL_INFO); 
    // LogComponentEnable("TcpSocketBase", LOG_LEVEL_INFO);

    string typeStr = "ns3::UdpSocketFactory";
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    string protocol = "TCP"; 
    bool rttTrace = 0;

    if (tcp){
        tid = TypeId::LookupByName("ns3::TcpSocketFactory");
        typeStr = "ns3::TcpSocketFactory";
        protocol = "TCP";
        Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(1 << 22));
        Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1 << 22));
    }

    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(1 << 23)); // 8 MB
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1 << 23)); // 8 MB

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

    Config::SetDefault("ns3::BurstErrorModel::ErrorRate", DoubleValue(0.005));
    Config::SetDefault("ns3::BurstErrorModel::BurstSize",
                       StringValue("ns3::UniformRandomVariable[Min=1|Max=3]"));

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

    ApplicationContainer sourceApps; 
   
    double appStoptime = 205.0;
    double simStomtime = appStoptime + 0.25;
    Time startTime = MilliSeconds(1.0);
    Time AppStopTime = Seconds(appStoptime);
    Time simStopTime = Seconds(simStomtime);

    Ipv4Address remoteAddress = interfaces2.GetAddress(1);

    Ptr<Node> destNode = n2n3.Get(1);
    if (TNT) {
        remoteAddress = TNTinterfaces.GetAddress(1);
        destNode = n3nTnT.Get(1);
    }

    uint16_t packetSize = 5000; 
    std::string dataRate = "40Mb/s";
    uint16_t port = 49200;
    OnOffHelper onoff("ns3::TcpSocketFactory",
                      Address(InetSocketAddress(remoteAddress, port))); //remote address
    // onoff.SetConstantRate(DataRate(dataRate));
    onoff.SetAttribute("DataRate", StringValue(dataRate));
    onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=10]")); //switched from 10
    onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=1]")); //switched from 1 
    onoff.SetAttribute("PacketSize", UintegerValue(packetSize));
    sourceApps = onoff.Install(n1n2.Get(0));

    Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);

    sinkHelper.SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId()));
    ApplicationContainer sinkApps = sinkHelper.Install(destNode);


    //start source apps
    sourceApps.Start(startTime);
    sourceApps.Stop(AppStopTime);

    // uint16_t servPort = 4477;
    PacketSinkHelper sink(typeStr, InetSocketAddress(interfaces2.GetAddress(1)));
    // sinkApps = sink.Install(n2n3.Get(1));
    sinkApps.Start(startTime);
    sinkApps.Stop(AppStopTime); 


    if (tracing){

        firstRtt[0] = true;

        AsciiTraceHelper ascii;
        p2pUpload.EnableAsciiAll(ascii.CreateFileStream(prefix_file_name + ".tr"));
        // p2pUpload.EnablePcapAll(prefix_file_name, false);
        n1n2devices.Get(1)->TraceConnectWithoutContext("PhyRxDrop", MakeCallback(&RxDrop));
        
        // Ptr<Socket> recvSink = Socket::CreateSocket(n1n2.Get(1), tid); // node 1, receiver
        // recvSink->Connect(InetSocketAddress(interfaces.GetAddress(1), servPort));
        // recvSink->SetRecvCallback(MakeCallback(&ReceivePacket));  

        if (rttTrace){
            Simulator::Schedule(MilliSeconds(1 + 0.00001),
                                &TraceRtt,
                                prefix_file_name + "-rtt.data",
                                0); //index = 0

        }
        
    }

    // /home/drew/Thesis/baselinePcapData/ErrRate0.050/telemetryData-app2

    string dataFolder = "/home/drew/Thesis/baselinePcapData/ErrRate"+ErrorString+"/telemetryData-app2/three-node-tcp-OnOff_Run_"+runNumber;
        if (satSetup == "LEO"){
            dataFolder = "/home/drew/Thesis/baselinePcapData/LEO/ErrRate"+ErrorString+"/telemetryData-app2/three-node-tcp-OnOff_Run_"+runNumber;
        }

    p2pDownload.EnablePcap(dataFolder, 3, 0, false);

    std::string tntEnable = "Disabled";
    if (TNT) {
        tntEnable = "Enabled";
    }

    auto apps = sinkApps; //sinkApps;
    apps.Start(startTime);
    uint32_t i = 0;
    std::vector<Ptr<ClientRxTracePlot>> plots;
    for (auto app = apps.Begin(); app != apps.End(); app++, i++)
    {
        std::stringstream ThreeNodeTCPOnOff;
        ThreeNodeTCPOnOff << "TCP" <<"-OnOff-" << "ClientRxPlot"<<"TNT-"<<tntEnable <<"RtnDrop-"<<rtnDropEnable;
        plots.push_back(CreateObject<ClientRxTracePlot>(*app, ThreeNodeTCPOnOff.str()));
    }

    NS_LOG_INFO("Starting three-node-tcp-OnOff...");
    NS_LOG_INFO("L4 Protocol: " << protocol);
    if (EMM)
    {
    NS_LOG_INFO("Error Model Enabled");
    NS_LOG_INFO("TNT: " << tntEnable);  
    NS_LOG_INFO("Return Link Drop: " << rtnDropEnable);
    // NS_LOG_INFO("Error Model Type:" << errType);
    NS_LOG_INFO("Error Amount: " << errorRate);
    NS_LOG_INFO("Packet Size: " << packetSize); 
    NS_LOG_INFO("Data Rate: " << dataRate);
    NS_LOG_INFO("TCP Buffer Sizes: " << "8MB");
    } else {
        NS_LOG_INFO("No Error Model Enabled");
    }

    //get node id of n1n2.Get(1)
    uint16_t nodeId = n3nTnT.Get(1)->GetId();
    NS_LOG_INFO("Node ID: " << nodeId);

    //get device ID of n1n2.Get(1)
    uint16_t deviceID = TNTdevices.Get(1)->GetIfIndex();
    NS_LOG_INFO("Device ID: " << deviceID);

    bool prom = false;
    string pcapName = "three-node-OnOff"; 

    if (prom){
        pcapName = "three-node-OnOff-prom";
    }
    // p2pDownload.EnablePcapAll(simName);

    // p2pUpload.EnablePcap("/Users/drewspitzer/ns-3-dev/scratch/pythonParse/OnOff/three-node-OnOff", nodeId, deviceID, prom);
    n1n2devices.Get(1)->TraceConnectWithoutContext("PhyRxDrop", MakeCallback(&RxDrop));
        
    // p2pDownload.EnablePcapAll("three-node-OnOff");


////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

        Ptr<PacketSink> sink1 = DynamicCast<PacketSink>(sinkApps.Get(0));
        uint32_t totalRx = sink1->GetTotalRx(); 
        std::cout << "Three-Nodes TCP OnOff Test" << std::endl;
        std::cout << "\nTotal Bytes Received: " << totalRx << std::endl;
        std::cout << "Calculated Throughput: " << totalRx * 8 / appStoptime / 1000 << " kbps\n" << std::endl;

        std::cout << "Set Upload Data Rate: " << uploadRate << endl;
        std::cout << "Set Download Data Rate: " << downloadRate << endl; 
        cout << "Set Ulink Delay: " << uploadDelay << endl; 
        cout << "Set Dlink Delay: " << downloadDelay << endl;
        std::cout << "Tota l RX Drops: " << totalRxDrop << std::endl; 
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

        string outputDir = "/home/drew/Thesis/satStatistics/three-node-tcp-OnOff";
        string simTag = "three-node-OnOff";//+"-"+errType+"-"+std::to_string(errorRate)+"-"+uploadRate+"-"+delay+".txt";
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
            "/home/drew/Thesis/satStatistics/three-node-tcp-OnOff/flowMonOutput-"+std::to_string(errorRate)+".xml", 
            true, true);

    }

    // Simulator::Destroy();
    return 0;


}
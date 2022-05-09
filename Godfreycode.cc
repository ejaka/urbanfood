
//ns version is 3.30.1
//two WiFi nodes using 1EEE802.11n Standard
//5GHz
//adhoc WiFi Mac
//output power of WiFi card of 10dBm
//transmitter and recieving gain of 1dBi
//UDP traffic
//Application layer packet size of 1450Bytes
//data-rate of 75Mbps.
//FriisProp=FriisPropagationLossModel
//FixedProp=FixedRssLossModel
//ThreeLogProp=ThreeLogDistancePropagationLossModel
//TwoRayProp=TwoRayGroundPropagationLossModel
//NakagamiProp=NakagamiPropagationLossModel
//


#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/on-off-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Adhoc WiFi network");

// Global variables for use in callbacks.
double g_signalDbmAvg;
uint32_t g_samples;

void MonitorSniffRx (Ptr<const Packet> packet,
                     uint16_t channelFreqMhz,
                     WifiTxVector txVector,
                     MpduInfo aMpdu,
                     SignalNoiseDbm signalNoise)

{
  g_samples++;
  g_signalDbmAvg += ((signalNoise.signal - g_signalDbmAvg) / g_samples);

}
int main (int argc, char *argv[])
{
  std::string PropagationModels;
  double simulationTime = 70; //DEFAULT IN SECONDS
  double distance = 1.0;  // METRES
  uint32_t packetSize = 1450; // UDP PACKETS IN BYTE
  double datarate = 75.0; //IN Mbps
  double Width = 80.0; // THE CHANNEL WIDTH in MHz
  std::string CSVfileName = "AdhocWiFinetwork.csv";


  CommandLine cmd;
  cmd.AddValue ("PropagationModels", "FriisProp or FixedProps, ThreeLogProp, TwoRayProp, Nakagami PropagationModels", PropagationModels);
  cmd.AddValue ("distance", "distance (metres)", distance);
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);

  cmd.Parse (argc, argv);

  //To Create nodes
  NodeContainer nodes;
  nodes.Create (2);

  //Creating wifi NIC cards
  WifiHelper wifi;

  //Setting of Wifi standard for network
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  wifiPhy.Set ("Frequency", UintegerValue (5180)); //it assigns Frequency channel
  wifiPhy.Set ("TxPowerStart", DoubleValue(10.0)); //it assigns Transmission power start
  wifiPhy.Set ("TxPowerEnd", DoubleValue(10.0)); //it assigns Transmission power end
  wifiPhy.Set ("TxGain", DoubleValue(1.0)); //it assigns Transmitter Gain
  wifiPhy.Set ("RxGain", DoubleValue (1.0) ); //it assigns Receiver Gain
  wifiPhy.Set ("TxPowerLevels", UintegerValue (1)); //it assigns Power Levels
  wifiPhy.Set ("ChannelWidth", UintegerValue (Width)); //it assigns channel width


  //To enable tracing extension for the 802.11 standard
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);


  //To create wifi channel and set propagation
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  if (PropagationModels == "FriisProp")
  {
      wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  }
  else if (PropagationModels == "FixedProp")
   {
      wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel", "Rss",DoubleValue (-79));
   }
  else if (PropagationModels == "ThreeLogProp")
    {
      wifiChannel.AddPropagationLoss ("ns3::ThreeLogDistancePropagationLossModel");
    }
  else if (PropagationModels == "TwoRayProp")
    {
      wifiChannel.AddPropagationLoss ("ns3::TwoRayGroundPropagationLossModel");
    }
  else if (PropagationModels == "NakagamiProp")
    {
      wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");
    }

  else
    {
      std::cout << "Wrong propagation models!" << std::endl;
      return 0;
    }

  wifiPhy.SetChannel (wifiChannel.Create ());

  // Configure mac on the wifi devices
  WifiMacHelper wifiMac;

  wifi.SetRemoteStationManager ("ns3::MinstrelHtWifiManager");
  // According to requirement,Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes);

  //Setting  position of nodes
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.5));   //node 0
  positionAlloc->Add (Vector ((distance), 0.0, 0.5)); //node 1
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  //for internet stack and install on nodes
  InternetStackHelper internet;
  internet.Install (nodes);

  //Implement Ipv4 address helper for ip address assignment
  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

 /* Setting applications */

    // Create a server app on node one.
  //
  uint16_t port = 50000;
  UdpServerHelper serverA (port);
  ApplicationContainer serverAppA = serverA.Install (nodes.Get (1));
  serverAppA.Start (Seconds (0.0));
  serverAppA.Stop (Seconds (simulationTime + 1));


//
// OnOff applications to send data from node zero to node one.
//

  ApplicationContainer clientApp;
  OnOffHelper clientHelper ("ns3::UdpSocketFactory", Address ());
  AddressValue remoteAddress (InetSocketAddress (i.GetAddress (1), port));
  clientHelper.SetAttribute ("Remote", remoteAddress);
  clientHelper.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelper.SetAttribute ("PacketSize", UintegerValue (packetSize));
  clientHelper.SetAttribute ("DataRate", DataRateValue (datarate * 1000000)); //bit/s
  clientApp = clientHelper.Install (nodes.Get (0));
  clientApp.Start (Seconds (2.0));
  clientApp.Stop (Seconds (simulationTime));



  //for signal measurement
  Config::ConnectWithoutContext ("/NodeList/0/DeviceList/*/Phy/MonitorSnifferRx", MakeCallback (&MonitorSniffRx));
  g_signalDbmAvg = 0;
  g_samples = 0;


  /* Printing Output */

  //Installing of FlowMonitor on all nodes
  Ptr<FlowMonitor> monitor;

    FlowMonitorHelper flowmon;
    monitor = flowmon.InstallAll ();



  // Run simulation
  Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();
  


 // Print per flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  
  
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      //
      // Duration for throughput measurement is simulation time - 1 seconds, since
      //   StartTime of the OnOffApplication is at about "second 1"
      // and
      //   Simulator::Stops at default "70".

      if (i->first > 0)
        {

          Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
          std::cout << "Flow " << i->first - 2 << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
          std::cout << "  Propagation Model " << PropagationModels << "\n";
          std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
          std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          std::cout << "  Tx Offered:  " << i->second.txBytes * 8.0 / (simulationTime - 1) / 1000 / 1000  << " Mbps\n";
          std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
          std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
          std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (simulationTime - 1) / 1000 / 1000  << " Mbps\n";
          std::cout << "  Distance: " << distance << " m\n";
          std::cout << "  Simulation Time: " << simulationTime << " Sec\n";
          std::cout << "  Signal strength " << g_signalDbmAvg << " dBm\n";




          //To print out the flow monitor in csv form

          std::ofstream out (CSVfileName.c_str (), std::ios::app);
          out << "Flow " << i->first - 2 << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n"
          " Propagation Model " << PropagationModels << "\n"
          " Tx Packets: " << i->second.txPackets << "\n"
          " TxOffered:  " << i->second.txBytes * 8.0 / (simulationTime - 1) / 1000 / 1000  << " Mbps\n"
          " Rx Packets: " << i->second.rxPackets << "\n"
          " Rx Bytes:   " << i->second.rxBytes << "\n"
          " Throughput: " << i->second.rxBytes * 8.0 / (simulationTime - 1) / 1000 / 1000  << " Mbps\n"
          " Distance: " << distance << "\n"
          " Simulation Time: " << simulationTime << "\n" <<
          " Signal strength " << g_signalDbmAvg << "\n" <<

          std::endl;
          out.close ();


        }
    }
  Simulator::Destroy ();

  return 0;
}

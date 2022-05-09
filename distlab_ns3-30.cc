
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/on-off-helper.h"


// Network topology:
//
//  Wi-Fi 192.168.170.64/29
//
//    AP                  AP
//    * <-- distance -->  *
//    |                   |
//    n1                  n2
//
// Users may vary the following command-line arguments in addition to the
// attributes, global values, and default values typically available:
//
//    --simulationTime:  Simulation time in seconds [60]
//    --propagationModel", "Type of Propagation model (friis, fixed, three, two, or nak)", [propagationModel=friis];
//    --distance:        meters separation between nodes [10]
 
using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("AdhocWiFi_Network");

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
  uint32_t packetSize = 1450; // Udp packet size in bytes
  double rss = -50;  // -dBm
  double simulationTime = 60; //seconds
  double distance = 10.0; // metres
  double width = 160.0; // channel width in MHz
  double dataRate = 75.0; // Mbps
  bool verbose = false;
  std::string propagationModel("friis"); //
  std::string CSVfileName = "Adhoc_Wifi_p2pNet.csv";
  
  // To enable users supply parameters at command prompt
  CommandLine cmd;
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("propagationModel", "Type of Propagation model (friis, fixed, three, two, or naka)", propagationModel);
  cmd.AddValue ("distance", "distance between two nodes", distance);
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);

  cmd.Parse (argc, argv);
  Time::SetResolution (Time::NS);
  
  //Create nodes
  NodeContainer wifiApNode;
  wifiApNode.Create (2);
  
  //The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;

  //Set Wifi standard for network
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  
  // Turn on all Wifi logging
  if (verbose)
  {
       wifi.EnableLogComponents ();  // Turn on all Wifi logging
  }
  
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  wifiPhy.Set ("TxPowerStart", DoubleValue(10.0)); //assigns Transmission power start
  wifiPhy.Set ("TxPowerEnd", DoubleValue(10.0)); //assigns Transmission power end
  wifiPhy.Set ("TxGain", DoubleValue(1.0)); //assigns Transmitter Gain
  wifiPhy.Set ("RxGain", DoubleValue (1.0) ); //assigns Receiver Gain
  wifiPhy.Set ("ChannelWidth", UintegerValue (width)); //assigns channel width
  wifiPhy.Set ("Frequency", UintegerValue (5180)); //assigns Frequency channel
  wifiPhy.Set ("TxPowerLevels", UintegerValue (1)); //assigns Power Levels
  
  //Create wifi channel and set propagation
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  
  if (propagationModel == "friis")
  {
      wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
   }
  else if (propagationModel == "fixed")
  {
      wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel", "Rss", DoubleValue (rss));
   }
  else if(propagationModel == ("three"))
  {
      wifiChannel.AddPropagationLoss ("ns3::ThreeLogDistancePropagationLossModel");
   }
  else if(propagationModel == ("two"))
  {
      wifiChannel.AddPropagationLoss ("ns3::TwoRayGroundPropagationLossModel");
   }
  else if(propagationModel == ("nak"))
  {
      wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");
   }
  else 
  {
      std::cout << " The loss model is not given, enter a loss model " << "\n\n";
   }
  
  wifiPhy.SetChannel (wifiChannel.Create ());
  
  wifi.SetRemoteStationManager ("ns3::MinstrelHtWifiManager");
  
  // Configure mac on the wifi devices
  WifiMacHelper wifiMac;
  
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");

  NetDeviceContainer apDevice;
  apDevice = wifi.Install (wifiPhy, wifiMac, wifiApNode);
  
  /* Setting mobility model */
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  //Set position for APs
  positionAlloc->Add (Vector (0.0, 0.0, 1.0)); // node 0
  positionAlloc->Add (Vector (distance, 0.0, 1.0)); // node 1
  
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);
  
  /* Create internet stack */
  InternetStackHelper stack;
  stack.Install (wifiApNode);
  
  //Implement Ipv4 address helper for ip address assignment
  Ipv4AddressHelper address;
  address.SetBase ("192.168.170.64", "255.255.255.248");

  Ipv4InterfaceContainer apNodeInterface;
  apNodeInterface = address.Assign (apDevice);
  
  /* Setting UDP flow applications */
  uint16_t port = 40000;
  UdpServerHelper server (port);
  ApplicationContainer serverApp = server.Install (wifiApNode.Get (1));
  
  serverApp.Start (Seconds (0.0));
  serverApp.Stop (Seconds (simulationTime + 1));
 
//
// Create the OnOff applications to send data from node zero to node one.
//

  OnOffHelper onoff ("ns3::UdpSocketFactory", Address ());
  onoff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff.SetAttribute ("PacketSize", UintegerValue (packetSize));
  onoff.SetAttribute ("DataRate", DataRateValue (dataRate * 1000000)); //bit/s
  AddressValue remoteAddress (InetSocketAddress (apNodeInterface.GetAddress(1), port));
  onoff.SetAttribute ("Remote", remoteAddress);
  ApplicationContainer clientApp = onoff.Install (wifiApNode.Get (0));
  clientApp.Start (Seconds (3.1));
  clientApp.Stop (Seconds (simulationTime + 1));
  
  //Signal measurement
  Config::ConnectWithoutContext ("/NodeList/0/DeviceList/*/Phy/MonitorSnifferRx", MakeCallback (&MonitorSniffRx));
  g_signalDbmAvg = 0;
  g_samples = 0;
  
  //Install FlowMonitor on all nodes
  Ptr<FlowMonitor> monitor;

  FlowMonitorHelper flowMon;
  monitor = flowMon.InstallAll ();

  // Run simulation
  NS_LOG_INFO("Start Simulation.");
  Simulator::Stop (Seconds (simulationTime + 1));
  Simulator::Run ();
  
  // Print per flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowMon.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      //
      // Duration for throughput measurement is simulation time - 1 second, since
      //   StartTime of the OnOffApplication is at about "second 1"
     
      if (i->first > 0)
        {
          Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
          std::cout << " Flow: " << i->first - 2 << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
          std::cout << " Propagation Model: " << propagationModel << "\n";
          std::cout << " Tx Packets: " << i->second.txPackets << "\n";
          std::cout << " Tx Bytes:   " << i->second.txBytes << "\n";
          std::cout << " Tx Offered: " << i->second.txBytes * 8.0 / (simulationTime - 1) / 1000 / 1000  << " Mbps\n";
          std::cout << " Rx Packets: " << i->second.rxPackets << "\n";
          std::cout << " Rx Bytes:   " << i->second.rxBytes << "\n";
          std::cout << " Throughput: " << i->second.rxBytes * 8.0 / (simulationTime - 1) / 1000 / 1000  << " Mbps\n";
          std::cout << " Distance:   " << distance << " m\n";
          std::cout << " Simulation Time: " << simulationTime << " Sec\n";
          std::cout << " Signal strength: " << g_signalDbmAvg << " dBm\n";
                    
          //Print out the flow monitor in csv form

          std::ofstream out (CSVfileName.c_str (), std::ios::app);
          out << " Flow: " << i->first - 2 << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n"
          " Propagation Model: " <<  propagationModel << "\n"
          " Tx Packets: " << i->second.txPackets << "\n"
          " Tx Bytes:   " << i->second.txBytes << "\n"
          " Tx Offered:  " << i->second.txBytes * 8.0 / (simulationTime - 1) / 1000 / 1000  << " Mbps\n"
          " Rx Packets: " << i->second.rxPackets << "\n"
          " Rx Bytes:   " << i->second.rxBytes << "\n"
          " Throughput: " << i->second.rxBytes * 8.0 / (simulationTime - 1) / 1000 / 1000  << " Mbps\n"
          " Distance: " << distance << " m\n"
          " Simulation Time: " << simulationTime << "\n" <<
          " Signal strength " << g_signalDbmAvg << "\n" <<
          
          std::endl;
          out.close ();
        }
    }                     
  
  Simulator::Destroy ();
  NS_LOG_INFO("Simulation finished.");
  return 0;
}
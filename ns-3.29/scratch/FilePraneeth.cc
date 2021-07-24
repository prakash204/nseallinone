#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"



// client                    p2p1                 AP        p2p2                   server  
//   n0  n1  n2  n3  n4 ------------- n5  n6  n7  n8 ---------------  n9  n10  n11   n12
//   |   |   |   |   |   194.0.4.X    |   |   |   |     194.0.5.X     |    |    |     |
//  ===================               *   *   *   *                 ===================
//     194.0.1.X                         196.0.2.X                      194.0.3.X
      
     
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma1 = 4;
  uint32_t nCsma2 = 3;
  uint32_t nWifi = 3;
  bool tracing = true;

  CommandLine cmd;
  cmd.AddValue ("nCsma1", "Number of \"extra\" CSMA nodes/devices", nCsma1);
  cmd.AddValue ("nCsma2", "Number of \"extra\" CSMA nodes/devices", nCsma2);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc,argv);

  // The underlying restriction of 18 is due to the grid position
  // allocator's configuration; the grid layout will exceed the
  // bounding box if more than 18 nodes are provided.
  if (nWifi > 18)
    {
      std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
      return 1;
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  NodeContainer p2pNodes1,p2pNodes2;
  p2pNodes1.Create (2);//n4 n5
  p2pNodes2.Create (2);//n8 ,n9
  
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("2Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("1ms"));
  
   NetDeviceContainer p2pDevices1,p2pDevices2;
  p2pDevices1 = pointToPoint.Install (p2pNodes1);
  p2pDevices2 = pointToPoint.Install (p2pNodes2);
  
  NodeContainer csmaNodes1,csmaNodes2;
  csmaNodes1.Create (4);// n0 n1 n2 n3
  csmaNodes1.Add (p2pNodes1.Get (0));//n4
  csmaNodes2.Add (p2pNodes2.Get (1));//n8
  csmaNodes2.Create (3);// n9 n10 n11 n12
  
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("2Mbps"));
  csma.SetChannelAttribute ("Delay", StringValue ("1ms"));
  
  NetDeviceContainer csmaDevices1,csmaDevices2;
  csmaDevices1= csma.Install (csmaNodes1);
  csmaDevices2 = csma.Install (csmaNodes2);
  
  NodeContainer wifiStaNodes;
  wifiStaNodes.Add(p2pNodes1.Get(1));//n8
  wifiStaNodes.Create (2);//n6 n7

  NodeContainer wifiApNode = p2pNodes2.Get (0);//n

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());
  
  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
  
  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);
  
   mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);
  
   MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (wifiStaNodes);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);

  InternetStackHelper stack;
  stack.Install (csmaNodes1);
  stack.Install (csmaNodes2);
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);
  
  Ipv4AddressHelper address;

  address.SetBase ("194.0.4.0", "223.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces1;
  p2pInterfaces1 = address.Assign (p2pDevices1);

  address.SetBase ("194.0.1.0", "223.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces1;
  csmaInterfaces1 = address.Assign (csmaDevices1);

  address.SetBase ("194.0.2.0", "223.255.255.0");
  address.Assign (staDevices);
  address.Assign (apDevices);

  address.SetBase ("194.0.5.0", "223.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces2;
  p2pInterfaces2 = address.Assign (p2pDevices2);

  address.SetBase ("194.0.3.0", "223.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces2;
  csmaInterfaces2 = address.Assign (csmaDevices2);
  
  UdpEchoServerHelper echoServer (21);

  ApplicationContainer serverApps = echoServer.Install (csmaNodes2.Get (3));
  serverApps.Start (Seconds (0.0));
  serverApps.Stop (Seconds (500.0));

  UdpEchoClientHelper echoClient (csmaInterfaces2.GetAddress (3), 21);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (10000));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = 
   echoClient.Install (csmaNodes1.Get (0));
  clientApps.Start (Seconds (0.1));
  clientApps.Stop (Seconds (500.0));
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  AsciiTraceHelper ascii;
  pointToPoint.EnableAsciiAll(ascii.CreateFileStream("Trace5.tr"));
  NS_LOG_UNCOND ("Trace file is created"); 
  Simulator::Stop (Seconds (500.0));
  
  

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}


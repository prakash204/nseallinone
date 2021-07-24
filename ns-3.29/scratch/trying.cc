/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"

// Default Network Topology
//
//                                    Wifi 10.1.3.0
//                                                 AP
//                                   *    *    *    *
//                         10.1.2.0  |    |    |    |    10.1.1.0
// n12  n11  n10  n9  n8 ------------n5   n6   n7   n0 -------------- n1   n2   n3   n4
// |   |    |    |    |    p2p                              p2p        |    |    |    |
// ====================                                                ================
//   LAN 10.1.5.0                                                       LAN 10.1.4.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma2 = 3;
  uint32_t nCsma1 = 4;
  uint32_t nWifi = 2;
  bool tracing = false;

  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma1);
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
//create the p2p nodes
  NodeContainer p2pNodes1,p2pNodes2;
  p2pNodes1.Create (2);
  p2pNodes2.Create (2);//n0 and n1 will be created
  

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices1,p2pDevices2;
  p2pDevices1 = pointToPoint.Install (p2pNodes1);
  p2pDevices2 = pointToPoint.Install (p2pNodes2);

  NodeContainer csmaNodes1,csmaNodes2;
  csmaNodes1.Add (p2pNodes1.Get (1));
  csmaNodes1.Create (nCsma1);
  csmaNodes2.Add (p2pNodes2.Get (1));
  csmaNodes2.Create (nCsma2);

//p2pNodes.Get(0) represents the 0th node
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices1,csmaDevices2;
  csmaDevices1 = csma.Install (csmaNodes1);
  csmaDevices2 = csma.Install (csmaNodes2);

//wireless networks
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi); //n6,n7
  NodeContainer wifiApNode2 = p2pNodes2.Get (0);//n0
  NodeContainer wifiApNode1 = p2pNodes1.Get (0);//n5
//YANS - yet another network simulator
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());//communication path

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid"); //broadcast id
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices1,apDevices2;
  apDevices1 = wifi.Install (phy, mac, wifiApNode1);
  apDevices2 = wifi.Install (phy, mac, wifiApNode2);
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
  mobility.Install (wifiApNode1);
  mobility.Install (wifiApNode2);

  InternetStackHelper stack;
  stack.Install (csmaNodes1);
  stack.Install (csmaNodes2);
  stack.Install (wifiApNode1);
  stack.Install (wifiApNode2);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces1;
  p2pInterfaces1 = address.Assign (p2pDevices1);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces2;
  p2pInterfaces2 = address.Assign (p2pDevices2);

  address.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces1;
  csmaInterfaces1 = address.Assign (csmaDevices1);

  address.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces2;
  csmaInterfaces2 = address.Assign (csmaDevices2);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevices);
  address.Assign (apDevices1);
  address.Assign (apDevices2);




//UdpEchoServerHelper echoServer1 (16);
UdpEchoServerHelper echoServer2 (18);

/*  ApplicationContainer serverApps1 = echoServer1.Install (wifiStaNodes.Get(0));//n6 is server
  serverApps1.Start (Seconds (1.0));
  serverApps1.Stop (Seconds (360.0));

  UdpEchoClientHelper echoClient1 (Ipv4Address("10.1.3.1"), 16);
  echoClient1.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient1.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient1.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps1 = echoClient1.Install (csmaNodes1.Get (4));//n12
  clientApps1.Start (Seconds (2.0));
  clientApps1.Stop (Seconds (360.0));
*/
////////////////////////////////////////////////////////////////////////////

  ApplicationContainer serverApps2 = echoServer2.Install (csmaNodes2.Get(nCsma2));//n4 is server
  serverApps2.Start (Seconds (1.0));
  serverApps2.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient2 (csmaInterfaces2.GetAddress (nCsma2), 18);
  echoClient2.SetAttribute ("MaxPackets", UintegerValue (10));
  echoClient2.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient2.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps2 = echoClient2.Install (wifiStaNodes.Get (0));//n12
  clientApps2.Start (Seconds (2.0));
  clientApps2.Stop (Seconds (10.0));
///////////////////////////////////////////////////////////////////////////////////////////////////

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  Simulator::Stop (Seconds (10.0));

  /*if (tracing == false)
    {
      pointToPoint.EnablePcapAll ("third");
      phy.EnablePcap ("third", apDevices.Get (0));
      csma.EnablePcap ("third", csmaDevices.Get (0), true);
    }*/

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

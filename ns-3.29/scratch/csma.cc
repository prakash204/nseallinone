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
#include "ns3/netanim-module.h" //animation

//     n0   n1   n2
//     |     |    |
//     ============
//      LAN 192.168.1.0


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 3;

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

//container class

  NodeContainer csmaNodes;
  csmaNodes.Create (nCsma);

//Helper Class - sets the attributes

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

//Install the csma devices

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

//OSI Stack installation in the nodes

  InternetStackHelper stack;
  stack.Install (csmaNodes);

//Setting IP address
  
  Ipv4AddressHelper address;

  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

//UDP server

  UdpEchoServerHelper echoServer (50);

//Install UDP server to node 0

  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

//UDP client

  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (0), 50);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (3));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.50)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

//Install UDP client to node 2

  ApplicationContainer clientApps = echoClient.Install (csmaNodes.Get (2));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

// For Animation

 AnimationInterface anim ("csma.xml");
 anim.SetConstantPosition(csmaNodes.Get(0),1.0,20.0);
 anim.SetConstantPosition(csmaNodes.Get(1),30.0,40.0);
 anim.SetConstantPosition(csmaNodes.Get(2),40.0,50.0);

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

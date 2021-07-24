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
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h" // for animation

#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"



using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
//create Nodes
  NodeContainer nodes;
  nodes.Create (2); //Node ID's 0 and 1

//Communication Channel
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

//Install Devices
  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

//Install Protocol Stack
  InternetStackHelper stack;
  stack.Install (nodes);

//Assign Address IPv4
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0"); //10.1.1.1 (0) 10.1.1.2 (1)

  Ipv4InterfaceContainer interfaces = address.Assign (devices);

//Application UDP Server
  UdpEchoServerHelper echoServer (9); //9 Port Number

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1)); //1 means second node
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

//Application UDP client
  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (2));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

// For Network animation
 //AnimationInterface anim ("CNN1.xml");
 //anim.SetConstantPosition(nodes.Get(0),1.0,20.0);
 //anim.SetConstantPosition(nodes.Get(1),40.0,50.0);
 //NS_LOG_UNCOND("It is from my program");


//for Ascii tracing
pointToPoint.EnablePcapAll ("CNN"); //wireshark
 //AsciiTraceHelper ascii;
 //pointToPoint.EnableAsciiAll(ascii.CreateFileStream("CNN.tr"));
 //NS_LOG_UNCOND("Trace file has been created..");


//Run the simulation
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

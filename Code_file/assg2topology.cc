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
#include <fstream>
#include "ns3/core-module.h"

// Default Network Topology
//
//   Wifi 10.1.3.0
//                         
//  *    *    *    *   *   AP
//  |    |    |    |   |   10.1.1.0
// n3   n4   n5   n6   n7  n0     n1   n2   
//                         |      |     |
//                        ===============
//                        LAN 10.1.2.0

using namespace ns3;




NS_LOG_COMPONENT_DEFINE ("TcpExample");
//NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");


class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

/*static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  //NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}*/



int 
main (int argc, char *argv[])
{
	
  bool verbose = true;
  uint32_t nCsma = 2;
  uint32_t nWifi = 5;
  bool tracing = true;

  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc,argv);

  // The underlying restriction of 18 is due to the grid position
  // allocator's configuration; the grid layout will exceed the
  // bounding box if more than 18 nodes are provided.
  /*if (nWifi > 18)
    {
      std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
      return 1;
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    }*/
	
	
	
  //change these parameters for different simulations
  std::string bandwidth = "5Mbps";
  std::string delay = "5ms";
  double error_rate = 0.000001;
  //int queuesize = 10; //packets
  int simulation_time = 10; //seconds

  //TCP variant set to NewReno
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpNewReno::GetTypeId()));

  //set qsize
  //Config::SetDefault ("ns3::PfifoFastQueueDisc::SetMaxSize", UintegerValue(queuesize));
  Config::SetDefault ("ns3::QueueBase::MaxSize", StringValue ("10p"));
  
	 
  

  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue (bandwidth));
  pointToPoint.SetQueue ("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize ("10p"))); // p in 100p stands for packets
  
  pointToPoint.SetChannelAttribute ("Delay", StringValue (delay));


  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get(0));
  csmaNodes.Create (nCsma);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);
  

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  NodeContainer wifiApNode = p2pNodes.Get(0);

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
  
  
  
  Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  em->SetAttribute ("ErrorRate", DoubleValue (error_rate));
  p2pDevices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
  


  InternetStackHelper stack;
  stack.Install (p2pNodes.Get(1));
  stack.Install (csmaNodes);
  //stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);
  //std::cout<<p2pInterfaces.Get(1);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  //address.Assign (p2pNodes);
  address.Assign (staDevices);
  address.Assign (apDevices);
  
  

  uint16_t sinkPort = 8080;
  Address sinkAddress (InetSocketAddress (p2pInterfaces.GetAddress (1), sinkPort));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (p2pNodes.Get (1));
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (simulation_time));

  Ptr<MyApp> app1 = CreateObject<MyApp> ();
  Ptr<MyApp> app2 = CreateObject<MyApp> ();
  Ptr<MyApp> app3 = CreateObject<MyApp> ();
  Ptr<MyApp> app4 = CreateObject<MyApp> ();
  Ptr<MyApp> app5 = CreateObject<MyApp> ();
  Ptr<MyApp> app6 = CreateObject<MyApp> ();
  Ptr<MyApp> app7 = CreateObject<MyApp> ();

  AsciiTraceHelper asciiTraceHelper;
  AsciiTraceHelper ascii;
   
  int x; 
  std::cout << "Enter simulation case - 1 or 2 or 3 or 4: "; // Type a number and press enter
  std::cin >> x ;

  
  Ptr<Socket> ns3TcpSocket1 = Socket::CreateSocket (csmaNodes.Get (1), TcpSocketFactory::GetTypeId ());
  Ptr<Socket> ns3TcpSocket2 = Socket::CreateSocket (csmaNodes.Get (2), TcpSocketFactory::GetTypeId ()); 
  Ptr<Socket> ns3TcpSocket3 = Socket::CreateSocket (wifiStaNodes.Get(0), TcpSocketFactory::GetTypeId ());
  Ptr<Socket> ns3TcpSocket4 = Socket::CreateSocket (wifiStaNodes.Get(1), TcpSocketFactory::GetTypeId ());
  Ptr<Socket> ns3TcpSocket5 = Socket::CreateSocket (wifiStaNodes.Get(2), TcpSocketFactory::GetTypeId ());
  Ptr<Socket> ns3TcpSocket6 = Socket::CreateSocket (wifiStaNodes.Get(3), TcpSocketFactory::GetTypeId ());
  Ptr<Socket> ns3TcpSocket7 = Socket::CreateSocket (wifiStaNodes.Get(4), TcpSocketFactory::GetTypeId ());
 // app1->Setup (ns3TcpSocket, sinkAddress, 1460, 1000000, DataRate ("100Mbps"));
 

  switch(x) {
  case 1:
    // code block
  //ns3TcpSocket1 = Socket::CreateSocket (p2pNodes.Get (1), TcpSocketFactory::GetTypeId ());
  app1->Setup (ns3TcpSocket1, sinkAddress, 1460, 1000000, DataRate ("100Mbps"));
  csmaNodes.Get (1)->AddApplication (app1);
  app1->SetStartTime (Seconds (1.));
  app1->SetStopTime (Seconds (simulation_time));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  
  csma.EnablePcap ("csma_case1", csmaDevices.Get (1), true);
 
  pointToPoint.EnablePcapAll ("p2p_case1");


  //detailed trace of queue enq/deq packet tx/rx
  pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("tcp-example_case1.tr"));
 
  csma.EnableAsciiAll (ascii.CreateFileStream ("tcp-example_csma_case1.tr"));
  
  break;
   
  case 2:
   
  app1->Setup (ns3TcpSocket1, sinkAddress, 1460, 1000000, DataRate ("200Mbps"));
  app2->Setup (ns3TcpSocket2, sinkAddress, 1461, 1000000, DataRate ("200Mbps")); 

  csmaNodes.Get(1)->AddApplication (app1);
  csmaNodes.Get(2)->AddApplication (app2);

  app1->SetStartTime (Seconds (1.));
  app1->SetStopTime (Seconds (simulation_time));
  app2->SetStartTime (Seconds (1.));
  app2->SetStopTime (Seconds (simulation_time));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  //csma.EnablePcap ("csma_case2", csmaDevices.Get (0), true);
  csma.EnablePcap ("csma_case2", csmaDevices.Get (1), true);
  csma.EnablePcap ("csma_case2", csmaDevices.Get (2), true);
  pointToPoint.EnablePcapAll ("p2p_case2");
  //phy.EnablePcapAll ("phy_case2");

  //detailed trace of queue enq/deq packet tx/rx
  pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("tcp-example_case2.tr"));
  //phy.EnableAsciiAll (ascii.CreateFileStream ("tcp-example_wifi_case2.tr"));
  csma.EnableAsciiAll (ascii.CreateFileStream ("tcp-example_csma_case2.tr"));
 
  break;

  case 3:
  

  app1->Setup (ns3TcpSocket1, sinkAddress, 1460, 1000000, DataRate ("500Mbps"));
  app2->Setup (ns3TcpSocket2, sinkAddress, 1461, 1000000, DataRate ("500Mbps"));
  app3->Setup (ns3TcpSocket3, sinkAddress, 1462, 1000000, DataRate ("500Mbps"));
  app4->Setup (ns3TcpSocket4, sinkAddress, 1463, 1000000, DataRate ("500Mbps"));
  app5->Setup (ns3TcpSocket5, sinkAddress, 1464, 1000000, DataRate ("500Mbps"));

  //app->Setup (ns3TcpSocket, sinkAddress, 1460, 1000000, DataRate ("100Mbps"));
  csmaNodes.Get (1)->AddApplication (app1);
  csmaNodes.Get (2)->AddApplication (app2);
  wifiStaNodes.Get(0)->AddApplication (app3);
  wifiStaNodes.Get(1)->AddApplication (app4);
  wifiStaNodes.Get(2)->AddApplication (app5);

  app1->SetStartTime (Seconds (1.));
  app1->SetStopTime (Seconds (simulation_time));
  app2->SetStartTime (Seconds (1.));
  app2->SetStopTime (Seconds (simulation_time));
  app3->SetStartTime (Seconds (1.));
  app3->SetStopTime (Seconds (simulation_time));
  app4->SetStartTime (Seconds (1.));
  app4->SetStopTime (Seconds (simulation_time));
  app5->SetStartTime (Seconds (1.));
  app5->SetStopTime (Seconds (simulation_time));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
 

  //csma.EnablePcap ("csma_case3", csmaDevices.Get (0), true);
  csma.EnablePcap ("csma_case3", csmaDevices.Get (1), true);
  csma.EnablePcap ("csma_case3", csmaDevices.Get (2), true);
  pointToPoint.EnablePcapAll ("p2p_case3");
  //pointToPoint.EnablePcap ("p2p_case3", p2pDevices.Get (0), true);
  //pointToPoint.EnablePcap ("p2p_case3", p2pDevices.Get (1), true);
  phy.EnablePcap ("phy_case3", staDevices.Get (0), true);
  phy.EnablePcap ("phy_case3", staDevices.Get (1), true);
  phy.EnablePcap ("phy_case3", staDevices.Get (2), true);

  //pointToPoint.EnablePcapAll ("p2p_case3");
  //phy.EnablePcapAll ("phy_case3");

  //detailed trace of queue enq/deq packet tx/rx
  pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("tcp-example_case3.tr"));
  phy.EnableAsciiAll (ascii.CreateFileStream ("tcp-example_wifi_case3.tr"));
  csma.EnableAsciiAll (ascii.CreateFileStream ("tcp-example_csma_case3.tr"));

  break;

  case 4:

  app1->Setup (ns3TcpSocket1, sinkAddress, 1460, 1000000, DataRate ("700Mbps"));
  app2->Setup (ns3TcpSocket2, sinkAddress, 1461, 1000000, DataRate ("700Mbps"));
  app3->Setup (ns3TcpSocket3, sinkAddress, 1462, 1000000, DataRate ("700Mbps"));
  app4->Setup (ns3TcpSocket4, sinkAddress, 1463, 1000000, DataRate ("700Mbps"));
  app5->Setup (ns3TcpSocket5, sinkAddress, 1464, 1000000, DataRate ("700Mbps"));
  app6->Setup (ns3TcpSocket6, sinkAddress, 1465, 1000000, DataRate ("700Mbps"));
  app7->Setup (ns3TcpSocket7, sinkAddress, 1466, 1000000, DataRate ("700Mbps"));


  csmaNodes.Get (1)->AddApplication (app1);
  csmaNodes.Get (2)->AddApplication (app2);
  wifiStaNodes.Get(0)->AddApplication (app3);
  wifiStaNodes.Get(1)->AddApplication (app4);
  wifiStaNodes.Get(2)->AddApplication (app5);
  wifiStaNodes.Get(3)->AddApplication (app6);
  wifiStaNodes.Get(4)->AddApplication (app7);


  app1->SetStartTime (Seconds (1.));
  app1->SetStopTime (Seconds (simulation_time));
  app2->SetStartTime (Seconds (1.));
  app2->SetStopTime (Seconds (simulation_time));
  app3->SetStartTime (Seconds (1.));
  app3->SetStopTime (Seconds (simulation_time));
  app4->SetStartTime (Seconds (1.));
  app4->SetStopTime (Seconds (simulation_time));
  app5->SetStartTime (Seconds (1.));
  app5->SetStopTime (Seconds (simulation_time));
  app6->SetStartTime (Seconds (1.));
  app6->SetStopTime (Seconds (simulation_time));
  app7->SetStartTime (Seconds (1.));
  app7->SetStopTime (Seconds (simulation_time));


  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  

  //csma.EnablePcap ("csma", csmaDevices.Get (0), true);
  csma.EnablePcap ("csma_case4", csmaDevices.Get (1), true);
  csma.EnablePcap ("csma_case4", csmaDevices.Get (2), true);
  //pointToPoint.EnablePcap ("p2p_case4", p2pDevices.Get (0), true);
  //pointToPoint.EnablePcap ("p2p_case4", p2pDevices.Get (1), true);
  pointToPoint.EnablePcapAll ("p2p_case4");
  phy.EnablePcap ("phy_case4", staDevices.Get (0), true);
  phy.EnablePcap ("phy_case4", staDevices.Get (1), true);
  phy.EnablePcap ("phy_case4", staDevices.Get (2), true);
  phy.EnablePcap ("phy_case4", staDevices.Get (3), true);
  phy.EnablePcap ("phy_case4", staDevices.Get (4), true);


  //pointToPoint.EnablePcapAll ("p2p_case4");
  //phy.EnablePcapAll ("phy_case4");

  //detailed trace of queue enq/deq packet tx/rx
  pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("tcp-example_case4.tr"));
  phy.EnableAsciiAll (ascii.CreateFileStream ("tcp-example_wifi_case4.tr"));
  csma.EnableAsciiAll (ascii.CreateFileStream ("tcp-example_csma_case4.tr"));

  break;

  default:
    // code block
  break;
}
  

  Simulator::Stop (Seconds (simulation_time));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
  
  
}

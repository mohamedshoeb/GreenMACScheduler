/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
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
 *
 * Author: Jaume Nin <jaume.nin@cttc.cat>
 */

#include "ns3/random-variable-stream.h"
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/netanim-module.h"
#include "ns3/packet-sink.h"
#include "ns3/gtk-config-store.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/gpm.h"

using namespace ns3;
using namespace std;

/************************************************************************************************************************************************

	Mohamed Shoeb & Ashraf Khresheh, April 2015

    This simulation runs an implementation of the green predictive scheduler
    specified in the following reference:

    Abou-zeid, H.; Hassanein, H.S., "Predictive green wireless access: exploiting mobility and application information," Wireless Communications,
    IEEE , vol.20, no.5, pp.92,99, October 2013

*************************************************************************************************************************************************

 Note: This simulation assumes very large eNb buffer, which we modified to be 1 Gigabyte in
 file source > lte > model > lte-rlc-um.cc in function LteRlcUm::GetTypeId

************************************************************************************************************************************************/

NS_LOG_COMPONENT_DEFINE ("EpcFirstExample");

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

NetDeviceContainer 		enbLteDevs;
NetDeviceContainer 		ueLteDevs;
Ptr<LteHelper> 			lteHelper;
NodeContainer           ueNodes;
NodeContainer           enbNodes;

void NotifyConnectionEstablishedUe (std::string context,
                               uint64_t imsi,
                               uint16_t cellid,
                               uint16_t rnti)
{
  std::cout	<< "\n ..............................................................................................................."
		    << "\n -- At " << Simulator::Now ().GetSeconds () << " Seconds\n   " << context
            << "\n -- UE IMSI " << imsi
            << ": connected to CellId " << cellid
            << " with RNTI " << rnti
		    << "\n ...............................................................................................................\n";
}

void NotifyHandoverStartUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellid,
                       uint16_t rnti,
                       uint16_t targetCellId)
{
  std::cout	<< "\n ..............................................................................................................."
		    << "\n -- At " << Simulator::Now ().GetSeconds () << " Seconds\n   " << context
            << "\n -- UE IMSI " << imsi
            << ": previously connected to CellId " << cellid
            << " with RNTI " << rnti
            << ", doing handover to CellId " << targetCellId
		    << "\n ...............................................................................................................\n";

}

void NotifyHandoverEndOkUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellid,
                       uint16_t rnti)
{
  std::cout	<< "\n ..............................................................................................................."
		    << "\n -- At " << Simulator::Now ().GetSeconds () << " Seconds\n   " << context
            << "\n -- UE IMSI " << imsi
            << ": successful handover to CellId " << cellid
            << " with RNTI " << rnti
		    << "\n ...............................................................................................................\n";
}

void NotifyConnectionEstablishedEnb (std::string context,
                                uint64_t imsi,
                                uint16_t cellid,
                                uint16_t rnti)
{
 std::cout	<< "\n ..............................................................................................................."
		    << "\n -- At " << Simulator::Now ().GetSeconds () << " Seconds\n   " << context
            << "\n -- eNB CellId " << cellid
            << ": successful connection of UE with IMSI " << imsi
            << " RNTI " << rnti
		    << "\n ...............................................................................................................\n";

}

void NotifyHandoverStartEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellid,
                        uint16_t rnti,
                        uint16_t targetCellId)
{

  std::cout	<< "\n ..............................................................................................................."
		    << "\n -- At " << Simulator::Now ().GetSeconds () << " Seconds\n   " << context
            << "\n -- eNB CellId " << cellid
            << ": start handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << " to CellId " << targetCellId
		    << "\n ...............................................................................................................\n";
}

void NotifyHandoverEndOkEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellid,
                        uint16_t rnti)
{

  std::cout	<< "\n ..............................................................................................................."
		    << "\n -- At " << Simulator::Now ().GetSeconds () << " Seconds\n   " << context
            << "\n -- eNB CellId " << cellid
            << ": completed handover of UE with IMSI " << imsi
            << " RNTI " << rnti
		    << "\n ...............................................................................................................\n";
}

void PrintDrop(void)
{

  cycle_sync = true;

  std::cout << "\n\n -- " << Simulator::Now().GetSeconds() << " Seconds Elapsed ---------------------------------------------------------------- \n";

  for (uint16_t i = 0; i < ueNodes.GetN(); i++)
  {

	  std::cout << "\n    [*] UE #" << ueNodes.Get(i)->GetId();

  }

  std::cout << "\n\n    [*] GPM Table has " << Table.size() << " record entries. \n";

  for (ii = Table.begin(); ii != Table.end(); ii++)
  {
	  std::cout << "\n    [*] Record Found : IMSI " << (*ii).imsi << ", RNTI " << (*ii).rnti << ", Current Cell ID " << (*ii).cellid_t << ", Video Length " << (*ii).video_length << " (bits) \n";
  }

  Simulator::Schedule(Seconds(1.0), &PrintDrop);

}

void printPosition(const Ptr<const MobilityModel> model)
{
    double distance;

	Ptr<Node> UE = model->GetObject<Node>(); // Get the current node using the mobility model

    distance = sqrt(pow(model->GetPosition().x ,2)+ pow(model->GetPosition().y ,2));

	std::cout << "\n    [*] printPosition: Time now is "  << Simulator::Now () << ", UE ID = " << (int)UE->GetId() << ", x = " << model->GetPosition().x << ", y = " << model->GetPosition().y << ", distance = " << distance;

}





int main (int argc, char *argv[])
{

	uint16_t                 i = 0;								// Global counter
	uint16_t eNb_numberOfNodes = 2;								// Number of eNodeBs
    uint16_t  UE_numberOfNodes = 3;								// Number of UEs

	double          eNb_Height = 60; 							// Antenna height in meters
	double            timestep = 1.0;							// Step size in seconds
	double        eNb_distance = 1200.0;						// Distance between any two eNodeBs in meters
	double    segment_Tx_Bytes = 512;							// Tx segment size
	double          startx_UE  = -500;							// X axis: Starting point of all UEs
	double          starty_UE  = 0;								// Y axis: Ground
	double          deltax_UE  = 5; 							// UE hopping distance in meters (distance of 1 hop at beginning of second)
	double             simTime = 2300 / deltax_UE;				// Simulation time in seconds

	cycle_slip_guard = 100;
	cycle_sync = false;

  // Command line arguments
  // CommandLine cmd;
  // cmd.AddValue("numberOfNodes", "Number of eNodeBs + UE pairs", numberOfNodes);
  // cmd.AddValue("simTime", "Total duration of the simulation [s])", simTime);
  // cmd.AddValue("distance", "Distance between eNBs [m]", distance);
  // cmd.AddValue("interPacketInterval", "Inter packet interval [ms])", interPacketInterval);
  // cmd.Parse(argc, argv);

  cout << "\n\n";
  cout << "__________________________________________________________________________________________________________________________________";
  cout << "\n * Simulation Start .. \n";
  cout << "__________________________________________________________________________________________________________________________________";
  cout << "\n";

  // LogComponentEnable("GreenFfMacScheduler", LOG_LEVEL_ALL);
  // LogComponentEnable("LteRlcUm", LOG_LEVEL_ALL);

  // ----------------------------------------------------------------------------------------------------------------------
  // Set eNB buffer size to a very large number to avoid overflow in predictive methods
  // ----------------------------------------------------------------------------------------------------------------------

	   Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (true));
	   Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue(25));
	   Config::SetDefault ("ns3::LteEnbNetDevice::UlBandwidth", UintegerValue(25));
       Config::SetDefault("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (1024 *1024 * 1024));
       Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (segment_Tx_Bytes));
       Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue (0));
       Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (1024 * 1024 * 1024));
       Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (1024 * 1024 * 1024));
       Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpTahoe")); // TcpTahoe, TcpNewReno, etc.
       // Disable HARQ
       Config::SetDefault ("ns3::GreenFfMacScheduler::HarqEnabled", BooleanValue (false));
       // CQI reports based on Mode 2: Mixed Mode (per subband)
       Config::SetDefault ("ns3::LteHelper::UsePdschForCqiGeneration", BooleanValue (false));

  // ----------------------------------------------------------------------------------------------------------------------
  // Create LTE Helper object and set LTE parameters
  // ----------------------------------------------------------------------------------------------------------------------

  lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  lteHelper->SetSchedulerType("ns3::GreenFfMacScheduler");
  lteHelper->SetHandoverAlgorithmType ("ns3::A2A4RsrqHandoverAlgorithm");
  lteHelper->SetHandoverAlgorithmAttribute ("ServingCellThreshold", UintegerValue (30));
  lteHelper->SetHandoverAlgorithmAttribute ("NeighbourCellOffset", UintegerValue (1));
  lteHelper->SetPathlossModelType("ns3::FriisPropagationLossModel");
  lteHelper->SetEnbAntennaModelType ("ns3::IsotropicAntennaModel");

  // ----------------------------------------------------------------------------------------------------------------------
  // TCP over Acknowledged Mode only (AM)
  // ----------------------------------------------------------------------------------------------------------------------

  // Ptr<LteEnbRrc> rrc = CreateObject<LteEnbRrc> ();
  // lteHelper>SetAttribute ("EpsBearerToRlcMapping", EnumValue (LteEnbRrc::RLC_AM_ALWAYS));

  // ----------------------------------------------------------------------------------------------------------------------
  // Uncomment to parse again so you can override default values from the command line
  // cmd.Parse(argc, argv);
  // ----------------------------------------------------------------------------------------------------------------------

  // ----------------------------------------------------------------------------------------------------------------------
  // Create a pointer to Packet Gateway
  // ----------------------------------------------------------------------------------------------------------------------

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  // ----------------------------------------------------------------------------------------------------------------------
  // Create RemoteHosts for all UEs
  // ----------------------------------------------------------------------------------------------------------------------

  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);
  
  // ----------------------------------------------------------------------------------------------------------------------
  // Create the Internet for P2P connection: RemoteHost & PGW
  // ----------------------------------------------------------------------------------------------------------------------

  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("1Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (512));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  // ----------------------------------------------------------------------------------------------------------------------
  // Create eNB and UE Nodes
  // ----------------------------------------------------------------------------------------------------------------------

  enbNodes.Create(eNb_numberOfNodes);
  ueNodes.Create(UE_numberOfNodes);
  
  // ----------------------------------------------------------------------------------------------------------------------
  // Install Mobility Model
  // ----------------------------------------------------------------------------------------------------------------------

  Ptr<ListPositionAllocator> node_positionAlloc = CreateObject<ListPositionAllocator> ();
  
  for (i = 0; i < eNb_numberOfNodes; i++)
    {
          node_positionAlloc->Add (Vector(eNb_distance * i, 0, eNb_Height));
    }

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(node_positionAlloc);
  mobility.Install(enbNodes);

  //************************************************************************UE mobility*********************************************************************

  mobility.SetMobilityModel("ns3::WaypointMobilityModel");
  mobility.Install(ueNodes);

  Ptr<WaypointMobilityModel>instant[UE_numberOfNodes];

  for (uint32_t nodeid = 0; nodeid < UE_numberOfNodes; nodeid++)
  {

  instant[nodeid] =  ueNodes.Get(nodeid)->GetObject<WaypointMobilityModel>();

  for(i = 1; i <= simTime; i++)
  {

  instant[nodeid]->AddWaypoint(Waypoint(Seconds(timestep), Vector(startx_UE, starty_UE, 0)));
  instant[nodeid]->AddWaypoint(Waypoint(Seconds(timestep+0.999999999), Vector(startx_UE, starty_UE, 0)));

  startx_UE = startx_UE + deltax_UE;

  timestep = timestep + 1.0; // 1 Second

  }

  startx_UE = -500.0 + (nodeid + 1) * 50;
  timestep  = 1.0;

  }

  Config::ConnectWithoutContext("/NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback(&printPosition));

  //***********************************************************************************************************************************************************

  // ----------------------------------------------------------------------------------------------------------------------
  // Install LTE Devices to the nodes
  // ----------------------------------------------------------------------------------------------------------------------
  
  enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  // ----------------------------------------------------------------------------------------------------------------------
  // Set eNB Tx Power
  // ----------------------------------------------------------------------------------------------------------------------

  	for(i = 0; i < eNb_numberOfNodes; i++)
  	{

  		Ptr<LteEnbPhy> eNbPhy = enbLteDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetPhy ();
  		eNbPhy->SetTxPower(0.0);
  		eNbPhy->SetNoiseFigure(5.0);
  	}

  // ----------------------------------------------------------------------------------------------------------------------
  // Install the IP stack on the UEs
  // ----------------------------------------------------------------------------------------------------------------------
  
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));

  // ----------------------------------------------------------------------------------------------------------------------
  // Attach all UEs to the first eNodeB at the beginning
  // ----------------------------------------------------------------------------------------------------------------------

  lteHelper->AttachToClosestEnb(ueLteDevs, enbLteDevs);

  // ----------------------------------------------------------------------------

  /* Youtube Video Bit Rates Table ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			           240p       360p        480p        720p        1080p
	Resolution      426 x 240   640 x 360   854x480     1280x720    1920x1080
	Video Bitrates
	Maximum         700 Kbps    1000 Kbps   2000 Kbps   4000 Kbps   6000 Kbps
	Recommended     400 Kbps    750 Kbps    1000 Kbps   2500 Kbps   4500 Kbps
	Minimum         300 Kbps    400 Kbps    500 Kbps    1500 Kbps   3000 Kbps

	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

  // Define Youtube Video Playback rate based on recommended settings

	double PBR1 =  400 * 1024 / 8;
	double PBR2 =  750 * 1024 / 8;
	double PBR3 = 1000 * 1024 / 8;
	double PBR4 = 2500 * 1024 / 8;
	double PBR5 = 3000 * 1024 / 8;

    // Select a video length for this UE (randomly from pool)

    Ptr<UniformRandomVariable>rnd = CreateObject<UniformRandomVariable>();
    double   video_playback_size[ueNodes.GetN ()];
	double   possible_video_playback_rates[5] = {PBR1, PBR2, PBR3, PBR4, PBR5};
    uint32_t thisrnd, timernd;

	for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
	  {

		rnd->SetAttribute("Min", DoubleValue(1));
		rnd->SetAttribute("Max", DoubleValue(2));
		thisrnd = rnd->GetInteger()-1;

		rnd->SetAttribute("Min", DoubleValue(4));		// Minimum stored video duration in minutes
		rnd->SetAttribute("Max", DoubleValue(10));		// Maximum stored video duration in minutes
		timernd = rnd->GetInteger();

		video_playback_size[u] = possible_video_playback_rates[thisrnd] * timernd * 60;

		Stored_Video_Playback_Rates.insert(Stored_Video_Playback_Rates.end(), possible_video_playback_rates[thisrnd]);
		Stored_Video_Playback_Times.insert(Stored_Video_Playback_Times.end(), timernd);
		Stored_Video_Playback_Sizes.insert(Stored_Video_Playback_Sizes.end(), video_playback_size[u]);

		std::cout << "\n    [*] UE #" << u << " requested stored video of size " << video_playback_size[u] << " and duration " << timernd << " minutes.";

	  }

  // ----------------------------------------------------------------------------------------------------------------------
  // Install and start applications on UEs and remote host
  // ----------------------------------------------------------------------------------------------------------------------
  
  uint16_t Port = 2000;

  ApplicationContainer sinkApps;
  ApplicationContainer sourceApps;
  


  Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();

  // TCP needs to be started late enough so that all UEs are connected
  // otherwise TCP SYN packets will get lost

  startTimeSeconds->SetAttribute ("Min", DoubleValue (1.00510));      // Must be greater than 1 second
  startTimeSeconds->SetAttribute ("Max", DoubleValue (1.01610));      // Must be greater than 1 second

  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
      {

	  // Assign IP address to UEs, and install applications

      Ptr<Node> ueNode = ueNodes.Get (u);

      // Set the default gateway for the UE

      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    
      // Increment DL Port

      ++Port;
      
      // Create Applications (Remote Host to UE)

      BulkSendHelper Source ("ns3::TcpSocketFactory", InetSocketAddress (ueIpIface.GetAddress(u), Port));

      // Set the amount of data to send in bytes.  Zero is unlimited.
      Source.SetAttribute ("MaxBytes", UintegerValue (video_playback_size[u]));
      Source.SetAttribute ("SendSize", UintegerValue (1024 * 1024 * 512));  // Send a lot of data here to avoid zero LC allocations

      sourceApps.Add (Source.Install (remoteHost));

      PacketSinkHelper Sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), Port));
      
      sinkApps.Add (Sink.Install (ueNodes.Get(u)));

      Ptr<EpcTft> tft = Create<EpcTft> ();
      EpcTft::PacketFilter pf;
      pf.remotePortStart = Port;
      pf.remotePortEnd = Port;
      tft->Add (pf);
      EpsBearer bearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
      lteHelper->ActivateDedicatedEpsBearer (ueLteDevs.Get(u), bearer, tft);

      // randomize a bit start times to avoid simulation artifacts
      // (e.g., buffer overflows due to packet transmissions happening
      // exactly at the same time)

      Time startTime = Seconds (startTimeSeconds->GetValue ());

      std::cout << "\n    [*] Application start time " << startTime << endl;

      sourceApps.Start (startTime);
      sinkApps.Start (startTime);

      sinkApps.Stop(Seconds(simTime));
      sourceApps.Stop(Seconds(simTime));

      }
  
  // ----------------------------------------------------------------------------------------------------------------------
  // Uncomment to enable PCAP tracing
  // ----------------------------------------------------------------------------------------------------------------------

  // p2ph.EnablePcapAll("Green-Pro-first");

  Simulator::Schedule(Seconds(1.0), &PrintDrop);
  Simulator::Stop(Seconds(simTime));
  
  // AnimationInterface anim ("animation.xml");
  // anim.EnablePacketMetadata (true); // Note: Creates assert error after adding traces, X2 and Rx buffers

  // ----------------------------------------------------------------------------------------------------------------------
  // Add X2 interface
  // ----------------------------------------------------------------------------------------------------------------------

  lteHelper->AddX2Interface (enbNodes);

  // ----------------------------------------------------------------------------------------------------------------------
  // Enable all relevant Traces
  // ----------------------------------------------------------------------------------------------------------------------

  lteHelper->EnablePhyTraces ();
  lteHelper->EnableMacTraces ();
  lteHelper->EnableRlcTraces ();
  lteHelper->EnablePdcpTraces ();
  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats ();
  rlcStats->SetAttribute ("EpochDuration", TimeValue (Seconds (0.001)));
  Ptr<RadioBearerStatsCalculator> pdcpStats = lteHelper->GetPdcpStats ();
  pdcpStats->SetAttribute ("EpochDuration", TimeValue (Seconds (0.001)));

  // ----------------------------------------------------------------------------------------------------------------------
  // Connect custom trace sinks for RRC connection establishment and handover notification
  // ----------------------------------------------------------------------------------------------------------------------

  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/ConnectionEstablished", MakeCallback (&NotifyConnectionEstablishedEnb));
  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/ConnectionEstablished", MakeCallback (&NotifyConnectionEstablishedUe));
  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverStart", MakeCallback (&NotifyHandoverStartEnb));
  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/HandoverStart", MakeCallback (&NotifyHandoverStartUe));
  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverEndOk", MakeCallback (&NotifyHandoverEndOkEnb));
  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/HandoverEndOk", MakeCallback (&NotifyHandoverEndOkUe));

  // ----------------------------------------------------------------------------------------------------------------------
  // Flow Monitor
  // ----------------------------------------------------------------------------------------------------------------------

  // Ptr <FlowMonitor> flowmon;
  // FlowMonitorHelper flowmonhelper;
  // flowmon = flowmonhelper.Install (ueNodes);
  // flowmon = flowmonhelper.Install (remoteHost);

  // ----------------------------------------------------------------------------------------------------------------------
  // Start Simulation
  // ----------------------------------------------------------------------------------------------------------------------

//_____________________________________________________________________
//  Radio map
//_____________________________________________________________________

Ptr<RadioEnvironmentMapHelper> remHelper = CreateObject<RadioEnvironmentMapHelper> ();
remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/0"));
remHelper->SetAttribute ("OutputFile", StringValue ("rem.out"));
remHelper->SetAttribute ("XMin", DoubleValue (-400.0));
remHelper->SetAttribute ("XMax", DoubleValue (400.0));
remHelper->SetAttribute ("XRes", UintegerValue (100));
remHelper->SetAttribute ("YMin", DoubleValue (-300.0));
remHelper->SetAttribute ("YMax", DoubleValue (300.0));
remHelper->SetAttribute ("YRes", UintegerValue (75));
remHelper->SetAttribute ("Z", DoubleValue (0.0));
remHelper->SetAttribute ("UseDataChannel", BooleanValue (true));
remHelper->SetAttribute ("RbId", IntegerValue (10));
remHelper->Install ();


//_____________________________________________________________________

  Simulator::Run();

  // ----------------------------------------------------------------------------
  // Store flow monitor results
  // ----------------------------------------------------------------------------------------------------------------------

  // flowmon -> SerializeToXmlFile ("results.xml", true, true);

  // ----------------------------------------------------------------------------------------------------------------------
  // Uncomment if you wish to read default parameters from a file
  // ----------------------------------------------------------------------------------------------------------------------

  /*
  GtkConfigStore config;
  config.ConfigureAttributes();
  */

  // ----------------------------------------------------------------------------------------------------------------------
  // End simulation and destroy all
  // ----------------------------------------------------------------------------------------------------------------------

  Simulator::Destroy();

  cout << "\n\n";
  cout << "__________________________________________________________________________________________________________________________________";
  cout << "\n * Simulation Done .. \n";
  cout << "__________________________________________________________________________________________________________________________________";
  cout << "\n";

  return 0;

}

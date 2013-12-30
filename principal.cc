/*
  Fichero: principal.cc
  Descripción: aquí se implementará el main
  y la topología.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CsmaMulticastExample");

int
main ( int argc, char * argv[])
{
   Config::SetDefault ("ns3::CsmaNetDevice::EncapsulationMode", StringValue ("Dix"));

  // Allow the user to override any of the defaults at
  // run-time, via command-line arguments
  CommandLine cmd;
  cmd.Parse (argc, argv);

  NodeContainer acceso1;
  acceso1.Create (2);
  NodeContainer acceso2;
  acceso2.Create(2);
  NodeContainer troncal;
  troncal.Create(2);

  //Ptr<Node> Router1 = troncal.Get(0);  //Con esto podemos acceder a cada router
  //Ptr<Node> Router2 = troncal.Get(1);


  NS_LOG_INFO ("Topologia");
  //CsmaHelper para las redes de acceso
  CsmaHelper csma;
  csma_acceso.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
  csma_acceso.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  //PointToPointHelper para la red troncal
  PointToPointHelper point;

  csma_troncal.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
  csma_troncal.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  

  // We will use these NetDevice containers later, for IP addressing
  NetDeviceContainer ndacceso1 = csma.Install (acceso1);  // Primera red de acceso
  NetDeviceContainer ndacceso2 = csma.Install (acceso2);  // Segunda red de acceso
  NetDeviceContainer ndtroncal = point.Install (troncal);  // Red troncal


  NS_LOG_INFO ("Add IP Stack.");
  InternetStackHelper internet_acceso1;
  InternetStackHelper internet_acceso2;
  InternetStackHelper internet_troncal;
  internet_acceso1.Install (acceso1);
  internet_acceso2.Install (acceso2);
  internet_troncal.Install (troncal);


  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4Addr;
  ipv4Addr.SetBase ("10.1.1.0", "255.255.255.0");
  ipv4Addr.Assign (ndacceso1);
  ipv4Addr.SetBase ("10.1.2.0", "255.255.255.0");
  ipv4Addr.Assign (ndacceso2);
  ipv4Addr.SetBase ("10.1.3.0", "255.255.255.0");
  ipv4Addr.Assign (ndtroncal);
  
  //Aplicación ON OFF para probar la topología.
  uint16_t port=9;
  ApplicationContainer app = onoff.Install (acceso1.Get (0));
  app.Start (Seconds (1.0));
  app.Stop (Seconds (10.0));

  //Sumidero
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny(), port)));
  onoff.SetConstantRate (DataRate ("500kb/s"));

  app = sink.Install (acceso2.Get (0));
  app.Add (sink.Install (acceso2.Get(1)));
  app.Start (Seconds (1.0));
  app.Stop (Seconds (10.0));

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll (ascii.CreateFileStream ("csma-prueba.tr"));
  csma.EnablePcapAll ("csma-prueba", false);

  NS_LOG_INFO ("Run Simulation");
  Simulator::Run();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done");
  
  

}

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

  //unsigned nodos_acceso_1 = 2;
  //unsigned nodos_acceso_2 = 2;
  std::string data_rate_1   = "5Mbps";
  std::string data_rate_2   = "5Mbps";
  std::string data_rate_t   = "5Mbps";
  std::string delay_1       = "0.002";
  std::string delay_2       = "0.002";
  std::string delay_t       = "0.002";

  CommandLine cmd; // @FIXME
  //cmd.AddValue("NumeroNodosAcceso1", "Número de nodos en la red de acceso 1", nodos_acceso_1);
  //cmd.AddValue("NumeroNodosAcceso2", "Número de nodos en la red de acceso 2", nodos_acceso_2);
  cmd.AddValue("DataRate1",          "Capacidad de la red de acceso 1",       data_rate_1);
  cmd.AddValue("DataRate2",          "Capacidad de la red de acceso 2",       data_rate_2);
  cmd.AddValue("DataRatet",          "Capacidad de la red troncal",           data_rate_2);
  cmd.AddValue("Delay1",             "Retardo de la red de acceso 1",         delay_1);
  cmd.AddValue("Delay2",             "Retardo de la red de acceso 2",         delay_2);
  cmd.AddValue("Delayt",             "Retardo de la red troncal",             delay_t);
  cmd.Parse (argc, argv);

  NodeContainer nodos;
  nodos.Create(6);
 
  NodeContainer acceso1 = NodeContainer (nodos.Get (0), nodos.Get (1), nodos.Get(2));
  NodeContainer acceso2 = NodeContainer (nodos.Get(3), nodos.Get(4), nodos.Get(5));
  NodeContainer troncal = NodeContainer (nodos.Get(2), nodos.Get(3));

  //Ptr<Node> Router1 = troncal.Get(0);  //Con esto podemos acceder a cada router
  //Ptr<Node> Router2 = troncal.Get(1);

  NS_LOG_INFO ("Creando Topologia");
  // Redes de acceso
  CsmaHelper csma_acceso1,csma_acceso2;
  csma_acceso1.SetChannelAttribute ("DataRate", DataRateValue (DataRate (data_rate_1)));
  csma_acceso1.SetChannelAttribute ("Delay", StringValue (delay_1));
  csma_acceso2.SetChannelAttribute ("DataRate", DataRateValue (DataRate (data_rate_2)));
  csma_acceso2.SetChannelAttribute ("Delay", StringValue (delay_2));
  
  // Red troncal
  PointToPointHelper point;
  point.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (data_rate_t)));
  point.SetChannelAttribute ("Delay", StringValue (delay_t)); 
  
  // We will use these NetDevice containers later, for IP addressing
  NetDeviceContainer ndacceso1 = csma_acceso1.Install (acceso1);  // Primera red de acceso
  NetDeviceContainer ndacceso2 = csma_acceso2.Install (acceso2);  // Segunda red de acceso
  NetDeviceContainer ndtroncal = point.Install (troncal);         // Red troncal

  NS_LOG_INFO ("Add IP Stack.");
  InternetStackHelper internet;
  internet.Install (nodos);

  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4Addr;
  ipv4Addr.SetBase ("10.1.1.0", "255.255.255.0");
  ipv4Addr.Assign (ndacceso1);
  ipv4Addr.SetBase ("10.1.2.0", "255.255.255.0");
  ipv4Addr.Assign (ndacceso2);
  ipv4Addr.SetBase ("10.1.3.0", "255.255.255.0");
  ipv4Addr.Assign (ndtroncal);
  
  //Aplicación ON OFF para probar la topología.
  OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny(), 4566)));
  onoff.SetConstantRate (DataRate ("500kb/s"));
  
  ApplicationContainer app = onoff.Install (acceso1.Get (0));
  app.Start (Seconds (1.0));
  app.Stop (Seconds (10.0));

  //Sumidero
  uint16_t port=9;
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny(), port)));  

  AsciiTraceHelper ascii;
  // csma.EnableAsciiAll (ascii.CreateFileStream ("csma-prueba.tr"));
  csma_acceso1.EnablePcapAll ("csma-acceso1", false);
  csma_acceso2.EnablePcapAll ("csma-acceso2", false);

  NS_LOG_INFO ("Run Simulation");
  Simulator::Run();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done");
}

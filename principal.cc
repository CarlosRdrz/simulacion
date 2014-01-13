
/*
  Fichero: principal.cc
  Descripción: aquí se implementará el main
  y la topología.
*/

#include <iostream>
#include "trazas.h"
#include "topologia.h"
#include "navegador.h"
#include "transferencia.h"
#include "voip.h"

#define TSTUDENT 1.8331               //10 Simulaciones con 90% de intervalo de confianza
#define NUM_SIMULACIONES 10
#define TASA 5000000
#define T_INICIO 1
#define T_FINAL 10


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Principal");

int
main ( int argc, char * argv[])
{
  LogComponentEnable("Trazas", LOG_LEVEL_INFO);
  LogComponentEnable("Topologia", LOG_LEVEL_INFO);
  LogComponentEnable("Principal", LOG_LEVEL_INFO);

  Config::SetDefault ("ns3::CsmaNetDevice::EncapsulationMode", StringValue ("Dix"));

  bool tracing              = true;
  unsigned nodos_acceso     = 2;
  unsigned nodos_empresa    = 2;
  unsigned nodos_wifi       = 1;
  double distance           = 50.0;
  std::string data_rate_1   = "5Mbps";
  std::string data_rate_2   = "5Mbps";
  std::string data_rate_t   = "5Mbps";
  std::string delay_1       = "0.002";
  std::string delay_2       = "0.002";
  std::string delay_t       = "0.002";
  float tasa_errores        = 0.02;        
  int indice                = 0;   
  double tasa               = TASA;
  double tiempo             = T_FINAL-T_INICIO;
  double uso_enlace         = 0;      //Porcentaje de uso del enlace que devolverá el método ImprimeTrazas
  Average<double> acumulador_uso;  
  double intervalo          = 0;
  // Puertos
  uint16_t http_port        = 16500;
  uint16_t udp_port         = 16600;
  uint16_t ftp_port         = 16700;
  uint16_t http_wifi_port   = 16800;

  CommandLine cmd;
  cmd.AddValue("NumeroNodosAcceso",  "Número de nodos en la red de acceso 1", nodos_acceso);
  cmd.AddValue("NumeroNodosEmpresa", "Número de nodos en la red de la empresa", nodos_empresa);
  cmd.AddValue("NumeroNodosWifi",    "Número de nodos que usan wifi",         nodos_wifi);
  cmd.AddValue("DataRate1",          "Capacidad de la red de acceso 1",       data_rate_1);
  cmd.AddValue("DataRate2",          "Capacidad de la red de acceso 2",       data_rate_2);
  cmd.AddValue("DataRatet",          "Capacidad de la red troncal",           data_rate_2);
  cmd.AddValue("Delay1",             "Retardo de la red de acceso 1",         delay_1);
  cmd.AddValue("Delay2",             "Retardo de la red de acceso 2",         delay_2);
  cmd.AddValue("Delayt",             "Retardo de la red troncal",             delay_t);
  cmd.Parse (argc, argv);

//for(nodos_acceso = 2; nodos_acceso = 5; nodos_acceso++)
 //{
  for(tasa_errores = 0.005; tasa_errores <= 0.015; tasa_errores += 0.005)
  {
    NS_LOG_INFO("LA TASA DE ERRORES ES: " << tasa_errores);
    acumulador_uso.Reset ();
    for (indice = 0; indice <= NUM_SIMULACIONES; indice++)
    {
      // Variables del sistema
      Trazas traza;
      Topologia topologia;

      // Añadimos los contenedores de nodos
      NS_LOG_INFO ("Creando Topologia");
      topologia.AddContainer ("troncal", 2);
      topologia.AddContainer ("acceso", nodos_acceso);
      topologia.AddContainer ("empresa", nodos_empresa);
      topologia.AddContainer ("wifi", nodos_wifi);

      // Añadimos los routers para formar las subredes
      topologia.AddNodeToContainer("troncal", 0, "acceso");
      topologia.AddNodeToContainer("troncal", 1, "empresa");
      topologia.AddNodeToContainer("troncal", 0, "wifi");

      // Creamos las redes PointToPoint, CSMA y Wifi
      topologia.AddPPPNetwork("troncal", data_rate_t, delay_t);
      topologia.AddCsmaNetwork("acceso", data_rate_1, delay_1);
      topologia.AddCsmaNetwork("empresa", data_rate_2, delay_2);
      topologia.AddWifiNetwork("wifi");

      NS_LOG_INFO ("Añadimos el stack IP");
      topologia.BuildInternetStack ();

      NS_LOG_INFO ("Asignamos direcciones IP.");
      topologia.SetIpToNetwork ("acceso", "10.1.1.0", "255.255.255.0");
      topologia.SetIpToNetwork ("empresa", "10.1.2.0", "255.255.255.0");
      topologia.SetIpToNetwork ("troncal", "10.1.3.0", "255.255.255.0");
      topologia.SetIpToNetwork ("wifi", "10.1.4.0", "255.255.255.0");

      // Popular tablas de routing
      Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

      // Añadimos el sistema de mobilidad a la red wifi
      topologia.AddMobility ("wifi", distance);
  
      // Añadimos las trazas a los routers de la troncal
      NS_LOG_INFO("Monitorizamos los dos routers");
      traza.Monitorize ("r0", topologia.GetNetDevice("troncal", 0));
      traza.Monitorize ("r1", topologia.GetNetDevice("troncal", 1));

      // Añadimos los modelos de errores
      topologia.SetErrorModel("troncal", tasa_errores);

      // Comenzamos a añadir aplicaciones...
      // Sumideros
      PacketSinkHelper sinkHttp ("ns3::TcpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny(), ++http_port)));
      PacketSinkHelper sinkUdp ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny(), ++udp_port)));
      PacketSinkHelper sinkFtp ("ns3::TcpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny(), ++ftp_port)));
      PacketSinkHelper sinkWifi ("ns3::TcpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny(), ++http_wifi_port)));
  
      ApplicationContainer sink1 = sinkHttp.Install (topologia.GetNode("empresa", 0));
      sink1.Start (Seconds (T_INICIO));
      sink1.Stop (Seconds (T_FINAL));

      ApplicationContainer sink2 = sinkUdp.Install (topologia.GetNode("empresa", 1)); 
      sink2.Start (Seconds (T_INICIO));
      sink2.Stop (Seconds (T_FINAL));

      ApplicationContainer sink3 = sinkFtp.Install (topologia.GetNode("empresa", 0));
      sink3.Start (Seconds (T_INICIO));
      sink3.Stop (Seconds (T_FINAL));

      ApplicationContainer sink4 = sinkWifi.Install (topologia.GetNode("empresa", 0));
      sink3.Start (Seconds (T_INICIO));
      sink3.Stop (Seconds (T_FINAL));

      // Navegador
      NavegadorHelper chrome (topologia.GetIPv4Address("empresa", 0), http_port);
      // Se instala la aplicación navegador
      ApplicationContainer navegador = chrome.Install (topologia.GetNode("acceso", 0));
      navegador.Start (Seconds (T_INICIO));
      navegador.Stop (Seconds (T_FINAL));
      
      // Navegador wifi
      NavegadorHelper safari (topologia.GetIPv4Address("empresa", 0), http_wifi_port);
      // Se instala la aplicación navegador en un dispositivo con wifi
      ApplicationContainer navegador_wifi = safari.Install (topologia.GetNode("wifi", 0));
      navegador_wifi.Start (Seconds (T_INICIO));
      navegador_wifi.Stop (Seconds (T_FINAL));
        
      // Telefono IP
      VoipHelper ciscoPhone (topologia.GetIPv4Address("empresa", 1), udp_port);
      ApplicationContainer app_voip = ciscoPhone.Install (topologia.GetNode("acceso", 1));
      // Se instala la aplicacion Voip
      app_voip.Start (Seconds (T_INICIO));
      app_voip.Stop (Seconds (T_FINAL));
   
      //Transferencia fichero
      TransferenciaHelper ftp (topologia.GetIPv4Address("empresa", 0), ftp_port);
      // Se instala la aplicación transferencia
      ApplicationContainer transferencia = ftp.Install (topologia.GetNode("acceso", 0));
      transferencia.Start (Seconds(T_INICIO));
      transferencia.Stop (Seconds(T_FINAL));

      // Activamos la creacion de archivos PCAPs
      if(tracing) {
        topologia.EnablePCAPLogging ("acceso");
        topologia.EnablePCAPLogging ("empresa");
      }

      NS_LOG_INFO ("Ejecutando simulacion...");
      Simulator::Run();
      // Imprimimos todas las trazas monitorizadas
      uso_enlace = traza.ImprimeTrazas(tasa,tiempo); 
      acumulador_uso.Update(uso_enlace);
      NS_LOG_INFO("El uso del enlace es: " << uso_enlace << "%");
      Simulator::Destroy ();
      NS_LOG_INFO ("Done");
    }
      intervalo = sqrt(acumulador_uso.Var() / NUM_SIMULACIONES) * TSTUDENT;
      NS_LOG_INFO ("El intervalo de confianza es: " << intervalo);
  }
 //}
}

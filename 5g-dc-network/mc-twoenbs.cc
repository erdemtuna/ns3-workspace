/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/* *
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
 * Author: Michele Polese <michele.polese@gmail.com>
 */

#include "ns3/mmwave-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/lte-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/config-store-module.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
//#include "ns3/gtk-config-store.h"
#include <ns3/buildings-helper.h>
#include <ns3/buildings-module.h>
#include <ns3/random-variable-stream.h>
#include <ns3/lte-ue-net-device.h>
#include "ns3/netanim-module.h"
#include "ns3/lte-helper.h"

#include <iostream>
#include <ctime>
#include <stdlib.h>
#include <list>


using namespace ns3;

/**
 * Sample simulation script for MC device. It instantiates a LTE and two MmWave eNodeB,
 * attaches one MC UE to both and starts a flow for the UE to and from a remote host.
 */

NS_LOG_COMPONENT_DEFINE ("McTwoEnbs");

/**********************************************************************/
void
NotifyHandoverStartUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellid,
                       uint16_t rnti,
                       uint16_t targetCellId)
{
  std::cout << Simulator::Now ().GetSeconds () << context
            << " UE IMSI " << imsi
            << ": previously connected to CellId " << cellid
            << " with RNTI " << rnti
            << ", doing handover to CellId " << targetCellId
            << std::endl;
}

void
NotifyHandoverEndOkUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellid,
                       uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << context
            << " UE IMSI " << imsi
            << ": successful handover to CellId " << cellid
            << " with RNTI " << rnti
            << std::endl;
}

void
NotifyConnectionEstablishedEnb (std::string context,
                                uint64_t imsi,
                                uint16_t cellid,
                                uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << context
            << " eNB CellId " << cellid
            << ": successful connection of UE with IMSI " << imsi
            << " RNTI " << rnti
            << std::endl;
}

void
NotifyHandoverStartEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellid,
                        uint16_t rnti,
                        uint16_t targetCellId)
{
  std::cout << Simulator::Now ().GetSeconds () << context
            << " eNB CellId " << cellid
            << ": start handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << " to CellId " << targetCellId
            << std::endl;
}
void
NotifyConnectionEstablishedUe (std::string context,
                               uint64_t imsi,
                               uint16_t cellid,
                               uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << context
            << " UE IMSI " << imsi
            << ": connected to CellId " << cellid
            << " with RNTI " << rnti
            << std::endl;
}
void
NotifyHandoverEndOkEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellid,
                        uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << context
            << " eNB CellId " << cellid
            << ": completed handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << std::endl;
}
/**********************************************************************/

void 
PrintGnuplottableBuildingListToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  uint32_t index = 0;
  for (BuildingList::Iterator it = BuildingList::Begin (); it != BuildingList::End (); ++it)
    {
      ++index;
      Box box = (*it)->GetBoundaries ();
      outFile << "set object " << index
              << " rect from " << box.xMin  << "," << box.yMin
              << " to "   << box.xMax  << "," << box.yMax
              << " front fc \"gray\" "
              << std::endl;
    }
}

void 
PrintGnuplottableUeListToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteUeNetDevice> uedev = node->GetDevice (j)->GetObject <LteUeNetDevice> ();
          Ptr<MmWaveUeNetDevice> mmuedev = node->GetDevice (j)->GetObject <MmWaveUeNetDevice> ();
          Ptr<McUeNetDevice> mcuedev = node->GetDevice (j)->GetObject <McUeNetDevice> ();
          if (uedev)
            {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << "set label \"uedev" << uedev->GetImsi ()
					  << "\" at "<< pos.x << "," << pos.y << " left offset char 1,0 point pointtype 7 pointsize 3 lc rgb \"blue\""
					  << std::endl;
            }
          else if (mmuedev)
           {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << "set label \"mmuedev" << mmuedev->GetImsi ()
					  << "\" at "<< pos.x << "," << pos.y << " left offset char 1,0 point pointtype 7 pointsize 3 lc rgb \"blue\""
					  << std::endl;
            }
          else if (mcuedev)
           {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << "set label \"" << mcuedev->GetImsi ()
                      << "\" at "<< pos.x << "," << pos.y << " left offset char 1,0 point pointtype 7 pointsize 3 lc rgb \"blue\""
                      << std::endl;
            } 
        }
    }
}

void 
PrintGnuplottableEnbListToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteEnbNetDevice> enbdev = node->GetDevice (j)->GetObject <LteEnbNetDevice> ();
          Ptr<MmWaveEnbNetDevice> mmdev = node->GetDevice (j)->GetObject <MmWaveEnbNetDevice> ();
          if (enbdev)
            {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << "set label \"" << enbdev->GetCellId ()
                      << "\" at "<< pos.x << "," << pos.y
                      << " left offset char 1,0 point pointtype 7 pointsize 3 lc rgb \"brown\""
                      << std::endl;
            }
          else if (mmdev)
            {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << "set label \"" << mmdev->GetCellId ()
                      << "\" at "<< pos.x << "," << pos.y
                      << " left offset char 1,0 point pointtype 7 pointsize 3 lc rgb \"red\""
                      << std::endl;
            } 
        }
    }
}

void
ChangePosition(Ptr<Node> node, Vector vector)
{
  Ptr<MobilityModel> model = node->GetObject<MobilityModel> ();
  model->SetPosition(vector);
  NS_LOG_UNCOND("************************--------------------Change Position-------------------------------*****************");
}

void
ChangeSpeed(Ptr<Node> n, Vector speed)
{
  n->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (speed);
  NS_LOG_UNCOND("************************--------------------Change Speed-------------------------------*****************");
}

void
PrintPosition(Ptr<Node> node)
{
  Ptr<MobilityModel> model = node->GetObject<MobilityModel> ();
  NS_LOG_UNCOND("Position = " << model->GetPosition() << " at time " << Simulator::Now().GetSeconds());
}

void 
PrintLostUdpPackets(Ptr<UdpServer> app, std::string fileName)
{
  std::ofstream logFile(fileName.c_str(), std::ofstream::app);
  logFile << Simulator::Now().GetSeconds() << " " << app->GetLost() << std::endl;
  logFile.close();
  Simulator::Schedule(MilliSeconds(20), &PrintLostUdpPackets, app, fileName);
}


bool
AreOverlapping(Box a, Box b)
{
  return !((a.xMin > b.xMax) || (b.xMin > a.xMax) || (a.yMin > b.yMax) || (b.yMin > a.yMax) );
}


bool
OverlapWithAnyPrevious(Box box, std::list<Box> m_previousBlocks)
{
  for (std::list<Box>::iterator it = m_previousBlocks.begin(); it != m_previousBlocks.end(); ++it)
  {
    if (AreOverlapping(*it,box))
    {
      return true;
    }
  }
  return false;
}


static ns3::GlobalValue g_mmw1DistFromMainStreet("mmw1Dist", "Distance from the main street of the first MmWaveEnb",
    ns3::UintegerValue(50), ns3::MakeUintegerChecker<uint32_t>());
static ns3::GlobalValue g_mmw2DistFromMainStreet("mmw2Dist", "Distance from the main street of the second MmWaveEnb",
    ns3::UintegerValue(70), ns3::MakeUintegerChecker<uint32_t>());
static ns3::GlobalValue g_mmw3DistFromMainStreet("mmw3Dist", "Distance from the main street of the third MmWaveEnb",
    ns3::UintegerValue(110), ns3::MakeUintegerChecker<uint32_t>());
static ns3::GlobalValue g_mmWaveDistance("mmWaveDist", "Distance between MmWave eNB 1 and 2",
    ns3::UintegerValue(200), ns3::MakeUintegerChecker<uint32_t>());
static ns3::GlobalValue g_interPckInterval("interPckInterval", "Interarrival time of UDP packets (us)",
    ns3::UintegerValue(20), ns3::MakeUintegerChecker<uint32_t>());
static ns3::GlobalValue g_bufferSize("bufferSize", "RLC tx buffer size (MB)",
    ns3::UintegerValue(20), ns3::MakeUintegerChecker<uint32_t>());
static ns3::GlobalValue g_x2Latency("x2Latency", "Latency on X2 interface (us)",
    ns3::DoubleValue(500), ns3::MakeDoubleChecker<double>());
static ns3::GlobalValue g_mmeLatency("mmeLatency", "Latency on MME interface (us)",
    ns3::DoubleValue(10000), ns3::MakeDoubleChecker<double>());
static ns3::GlobalValue g_mobileUeSpeed("mobileSpeed", "The speed of the UE (m/s)",
    ns3::DoubleValue(10), ns3::MakeDoubleChecker<double>());
static ns3::GlobalValue g_fastSwitching("fastSwitching", "If true, use mc setup, else use hard handover",
    ns3::BooleanValue(true), ns3::MakeBooleanChecker());
static ns3::GlobalValue g_rlcAmEnabled("rlcAmEnabled", "If true, use RLC AM, else use RLC UM",
    ns3::BooleanValue(true), ns3::MakeBooleanChecker());
static ns3::GlobalValue g_runNumber ("runNumber", "Run number for rng",
    ns3::UintegerValue(10), ns3::MakeUintegerChecker<uint32_t>());
static ns3::GlobalValue g_outPath("outPath",
    "The path of output log files",
    ns3::StringValue("./"), ns3::MakeStringChecker());
static ns3::GlobalValue g_noiseAndFilter("noiseAndFilter", "If true, use noisy SINR samples, filtered. If false, just use the SINR measure",
    ns3::BooleanValue(false), ns3::MakeBooleanChecker());
static ns3::GlobalValue g_handoverMode("handoverMode",
    "Handover mode", ns3::UintegerValue(1), ns3::MakeUintegerChecker<uint8_t>());
static ns3::GlobalValue g_reportTablePeriodicity("reportTablePeriodicity", "Periodicity of RTs",
    ns3::UintegerValue(1600), ns3::MakeUintegerChecker<uint32_t>());
static ns3::GlobalValue g_outageThreshold("outageTh", "Outage threshold",
    ns3::DoubleValue(-5), ns3::MakeDoubleChecker<double>());
static ns3::GlobalValue g_lteUplink("lteUplink", "If true, always use LTE for uplink signalling",
    ns3::BooleanValue(false), ns3::MakeBooleanChecker());

int
main (int argc, char *argv[])
{
  LogLevel logLevel1 = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_LEVEL_ALL);
  //LogLevel logLevel2 = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_LEVEL_ALL);

  LogComponentEnable ("A2A4RsrqHandoverAlgorithm", logLevel1);
  LogComponentEnable ("EpcHelper", logLevel1);
  LogComponentEnable ("LteEnbRrc", logLevel1);
  LogComponentEnable ("MmWaveHelper", logLevel1);
  //LogComponentEnable ("MmWaveLteRrcProtocolReal", logLevel1);
  LogComponentEnable ("MmWaveUeNetDevice", logLevel1);
  //LogComponentEnable ("MmWaveBearerStatsConnector", logLevel);
  bool harqEnabled = true;
  bool fixedTti = false;
  unsigned symPerSf = 24;
  double sfPeriod = 100.0;
  unsigned scenario = 1;
  double ueHeight = 1.75;
  double simTime;
  double nXrooms = 5, nYrooms = 5;
  std::string netanim_scenario;
  std::string gnuplot_scenario_buildings, gnuplot_scenario_enbs, gnuplot_scenario_ues;
  //double UeFinaly;

  bool print = true;

  // Command line arguments
  CommandLine cmd;
  cmd.AddValue ("scenario", "Custom made scenario types:\n1- A general street scenario in a 160x80 map,\n2- A general street scenario in a 200x120 map,\n3- One mmWave and one LTE with two sources of blockage (100x15 map),\n4- One mmWave and one LTE with one source of blockage (100x15 map),\n5- Two mmWaves and one LTE two sources of blockage (100x15 map),\n6- Two mmWaves and one LTE one source of blockage (100x15 map),\n7- Corner turn case in 200x120 map.", scenario);
  cmd.Parse(argc, argv);
  NS_LOG_UNCOND("The simulation scenario is " << scenario << ".");

  UintegerValue uintegerValue;
  BooleanValue booleanValue;
  StringValue stringValue;
  DoubleValue doubleValue;
  //EnumValue enumValue;

  // Variables for the RT 
  int windowForTransient = 150; // number of samples for the vector to use in the filter
  GlobalValue::GetValueByName("reportTablePeriodicity", uintegerValue);
  int ReportTablePeriodicity = (int)uintegerValue.Get(); // in microseconds
  if(ReportTablePeriodicity == 1600)
  {
    windowForTransient = 150;
  }
  else if(ReportTablePeriodicity == 25600)
  {
    windowForTransient = 50;
  }
  else if(ReportTablePeriodicity == 12800)
  {
    windowForTransient = 100;
  }
  else
  {
    NS_ASSERT_MSG(false, "Unrecognized");
  }

  int vectorTransient = windowForTransient*ReportTablePeriodicity;

  /********** FS or HH mode **********/
  GlobalValue::GetValueByName("fastSwitching", booleanValue);
  bool fastSwitching = booleanValue.Get();
  bool hardHandover = !fastSwitching;
  /********** FS or HH mode **********/

  /********** parse params for RT, filter, HO mode **********/
  GlobalValue::GetValueByName("noiseAndFilter", booleanValue);
  bool noiseAndFilter = booleanValue.Get();
  GlobalValue::GetValueByName("handoverMode", uintegerValue);
  uint8_t hoMode = uintegerValue.Get();
  GlobalValue::GetValueByName("outageTh", doubleValue);
  double outageTh = doubleValue.Get();
  /********** params for RT, filter, HO mode **********/

  /********** parse params for RLC, X2, and mobileSpeed **********/
  GlobalValue::GetValueByName("rlcAmEnabled", booleanValue);
  bool rlcAmEnabled = booleanValue.Get();
  GlobalValue::GetValueByName("bufferSize", uintegerValue);
  uint32_t bufferSize = uintegerValue.Get();
  GlobalValue::GetValueByName("interPckInterval", uintegerValue);
  uint32_t interPacketInterval = uintegerValue.Get();
  GlobalValue::GetValueByName("x2Latency", doubleValue);
  double x2Latency = doubleValue.Get();
  GlobalValue::GetValueByName("mmeLatency", doubleValue);
  double mmeLatency = doubleValue.Get();
  GlobalValue::GetValueByName("mobileSpeed", doubleValue);
  double ueSpeed = doubleValue.Get();
  /********** params for RLC, X2, and mobileSpeed **********/



  NS_LOG_UNCOND("fastSwitching " << fastSwitching << "-" << " rlcAmEnabled " << rlcAmEnabled << "-" <<  " bufferSize " << bufferSize << "-" <<  " interPacketInterval " <<
      interPacketInterval << "-" <<  " x2Latency " << x2Latency << "-" << " mmeLatency " << mmeLatency << "-" << " mobileSpeed " << ueSpeed);

  // rng things
  GlobalValue::GetValueByName("runNumber", uintegerValue);
  uint32_t runSet = uintegerValue.Get();
  uint32_t seedSet = 5;
  RngSeedManager::SetSeed (seedSet);
  RngSeedManager::SetRun (runSet); 
  char seedSetStr[21];
  char runSetStr[21];
  sprintf(seedSetStr, "%d", seedSet);
  sprintf(runSetStr, "%d", runSet);

  /********** Naming Output Files **********/
  GlobalValue::GetValueByName("outPath", stringValue);
  std::string path = stringValue.Get();
  std::string mmWaveOutName = "MmWaveSwitchStats";
  std::string lteOutName = "LteSwitchStats";
  std::string dlRlcOutName = "DlRlcStats";
  std::string dlPdcpOutName = "DlPdcpStats";
  std::string ulRlcOutName = "UlRlcStats";
  std::string ulPdcpOutName = "UlPdcpStats";
  std::string  ueHandoverStartOutName =  "UeHandoverStartStats";
  std::string enbHandoverStartOutName = "EnbHandoverStartStats";
  std::string  ueHandoverEndOutName =  "UeHandoverEndStats";
  std::string enbHandoverEndOutName = "EnbHandoverEndStats";
  std::string cellIdInTimeOutName = "CellIdStats";
  std::string cellIdInTimeHandoverOutName = "CellIdStatsHandover";
  std::string mmWaveSinrOutputFilename = "MmWaveSinrTime";
  std::string x2statOutputFilename = "X2Stats";
  std::string udpSentFilename = "UdpSent";
  std::string udpReceivedFilename = "UdpReceived";
  std::string extension = ".txt";
  std::string version;
  if(fastSwitching)
  {
    version = "mc";
    Config::SetDefault ("ns3::MmWaveUeMac::UpdateUeSinrEstimatePeriod", DoubleValue (0));
  }
  else if(hardHandover)
  {
    version = "hh";
  }
  /********** Naming Output Files **********/

  //get current time
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];
  time (&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(buffer,80,"%d_%m_%Y_%I_%M_%S",timeinfo);
  std::string time_str(buffer);

  /********** Exporting Output Files **********/
  Config::SetDefault ("ns3::MmWaveHelper::RlcAmEnabled", BooleanValue(rlcAmEnabled));
  Config::SetDefault ("ns3::MmWaveHelper::HarqEnabled", BooleanValue(harqEnabled));
  Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::HarqEnabled", BooleanValue(harqEnabled));
  Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::HarqEnabled", BooleanValue(harqEnabled));
  Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::FixedTti", BooleanValue(fixedTti));
  Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::SymPerSlot", UintegerValue(6));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::ResourceBlockNum", UintegerValue(1));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::ChunkPerRB", UintegerValue(72));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::SymbolsPerSubframe", UintegerValue(symPerSf));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::SubframePeriod", DoubleValue(sfPeriod));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::TbDecodeLatency", UintegerValue(200.0));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::NumHarqProcess", UintegerValue(100));
  Config::SetDefault ("ns3::MmWaveBeamforming::LongTermUpdatePeriod", TimeValue (MilliSeconds (100.0)));
  Config::SetDefault ("ns3::LteEnbRrc::SystemInformationPeriodicity", TimeValue (MilliSeconds (5.0)));
  Config::SetDefault ("ns3::LteRlcAm::ReportBufferStatusTimer", TimeValue(MicroSeconds(100.0)));
  Config::SetDefault ("ns3::LteRlcUmLowLat::ReportBufferStatusTimer", TimeValue(MicroSeconds(100.0)));
  Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (320));
  Config::SetDefault ("ns3::LteEnbRrc::FirstSibTime", UintegerValue (2));
  Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::X2LinkDelay", TimeValue (MicroSeconds(x2Latency)));
  Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::X2LinkDataRate", DataRateValue(DataRate ("1000Gb/s")));
  Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::X2LinkMtu",  UintegerValue(10000));
  Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::S1uLinkDelay", TimeValue (MicroSeconds(1000)));
  Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::S1apLinkDelay", TimeValue (MicroSeconds(mmeLatency)));
  Config::SetDefault ("ns3::McStatsCalculator::MmWaveOutputFilename", StringValue                 (path + version + mmWaveOutName + "_" + seedSetStr + "_" + runSetStr + "_" + time_str + extension));
  Config::SetDefault ("ns3::McStatsCalculator::LteOutputFilename", StringValue                    (path + version + lteOutName    + "_" + seedSetStr + "_" + runSetStr + "_" + time_str + extension));
  Config::SetDefault ("ns3::McStatsCalculator::CellIdInTimeOutputFilename", StringValue           (path + version + cellIdInTimeOutName    + "_" + seedSetStr + "_" + runSetStr + "_" + time_str + extension));
  Config::SetDefault ("ns3::MmWaveBearerStatsCalculator::DlRlcOutputFilename", StringValue        (path + version + dlRlcOutName   + "_" + seedSetStr + "_" + runSetStr + "_" + time_str + extension));
  Config::SetDefault ("ns3::MmWaveBearerStatsCalculator::UlRlcOutputFilename", StringValue        (path + version + ulRlcOutName   + "_" + seedSetStr + "_" + runSetStr + "_" + time_str + extension));
  Config::SetDefault ("ns3::MmWaveBearerStatsCalculator::DlPdcpOutputFilename", StringValue       (path + version + dlPdcpOutName + "_" + seedSetStr + "_" + runSetStr + "_" + time_str + extension));
  Config::SetDefault ("ns3::MmWaveBearerStatsCalculator::UlPdcpOutputFilename", StringValue       (path + version + ulPdcpOutName + "_" + seedSetStr + "_" + runSetStr + "_" + time_str + extension));
  Config::SetDefault ("ns3::MmWaveBearerStatsConnector::UeHandoverStartOutputFilename", StringValue    (path + version +  ueHandoverStartOutName + "_" + seedSetStr + "_" + runSetStr + "_" + time_str + extension));
  Config::SetDefault ("ns3::MmWaveBearerStatsConnector::EnbHandoverStartOutputFilename", StringValue   (path + version + enbHandoverStartOutName + "_" + seedSetStr + "_" + runSetStr + "_" + time_str + extension));
  Config::SetDefault ("ns3::MmWaveBearerStatsConnector::UeHandoverEndOutputFilename", StringValue    (path + version +  ueHandoverEndOutName + "_" + seedSetStr + "_" + runSetStr + "_" + time_str + extension));
  Config::SetDefault ("ns3::MmWaveBearerStatsConnector::EnbHandoverEndOutputFilename", StringValue   (path + version + enbHandoverEndOutName + "_" + seedSetStr + "_" + runSetStr + "_" + time_str + extension));
  Config::SetDefault ("ns3::MmWaveBearerStatsConnector::CellIdStatsHandoverOutputFilename", StringValue(path + version + cellIdInTimeHandoverOutName + "_" + seedSetStr + "_" + runSetStr + "_" + time_str + extension));
  Config::SetDefault ("ns3::MmWaveBearerStatsConnector::MmWaveSinrOutputFilename", StringValue(path + version + mmWaveSinrOutputFilename + "_" + seedSetStr + "_" + runSetStr + "_" + time_str + extension));
  Config::SetDefault ("ns3::CoreNetworkStatsCalculator::X2FileName", StringValue                  (path + version + x2statOutputFilename    + "_" + seedSetStr + "_" + runSetStr + "_" + time_str + extension));
  //std::string lostFilename = path + version + "LostUdpPackets" +  "_" + seedSetStr + "_" + runSetStr + "_" + time_str + extension;
  //Config::SetDefault ("ns3::UdpServer::ReceivedPacketsFilename", StringValue(path + version + "ReceivedUdp" +  "_" + seedSetStr + "_" + runSetStr + "_" + time_str + extension));
  //Config::SetDefault ("ns3::UdpClient::SentPacketsFilename", StringValue(path + version + "SentUdp" +  "_" + seedSetStr + "_" + runSetStr + "_" + time_str + extension));
  //Config::SetDefault ("ns3::UdpServer::ReceivedSnFilename", StringValue(path + version + "ReceivedSn" +  "_" + seedSetStr + "_" + runSetStr + "_" + time_str + extension));
  Config::SetDefault ("ns3::LteRlcAm::BufferSizeFilename", StringValue(path + version + "RlcAmBufferSize" +  "_" + seedSetStr + "_" + runSetStr + "_" + time_str + extension));
  /********** Exporting Output Files **********/

  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (bufferSize * 1024 * 1024));
  Config::SetDefault ("ns3::LteRlcUmLowLat::MaxTxBufferSize", UintegerValue (bufferSize * 1024 * 1024));
  Config::SetDefault ("ns3::LteRlcAm::StatusProhibitTimer", TimeValue(MilliSeconds(10.0)));
  Config::SetDefault ("ns3::LteRlcAm::MaxTxBufferSize", UintegerValue (bufferSize * 1024 * 1024));

  /********** SCH TTT setting **********/
  switch(hoMode)
  {
    case 1:
        Config::SetDefault ("ns3::LteEnbRrc::SecondaryCellHandoverMode", EnumValue(LteEnbRrc::THRESHOLD));
        break;
    case 2:
        Config::SetDefault ("ns3::LteEnbRrc::SecondaryCellHandoverMode", EnumValue(LteEnbRrc::FIXED_TTT));
        break;
    case 3:
        Config::SetDefault ("ns3::LteEnbRrc::SecondaryCellHandoverMode", EnumValue(LteEnbRrc::DYNAMIC_TTT));
        break;
  }
  /********** SCH TTT setting **********/
  
  Config::SetDefault ("ns3::LteEnbRrc::FixedTttValue", UintegerValue (150));
  Config::SetDefault ("ns3::LteEnbRrc::CrtPeriod", IntegerValue (ReportTablePeriodicity));
  Config::SetDefault ("ns3::LteEnbRrc::OutageThreshold", DoubleValue (outageTh));
  Config::SetDefault ("ns3::MmWaveEnbPhy::UpdateSinrEstimatePeriod", IntegerValue (ReportTablePeriodicity));
  Config::SetDefault ("ns3::MmWaveEnbPhy::Transient", IntegerValue (vectorTransient));
  Config::SetDefault ("ns3::MmWaveEnbPhy::NoiseAndFilter", BooleanValue(noiseAndFilter));

  GlobalValue::GetValueByName("lteUplink", booleanValue);
  bool lteUplink = booleanValue.Get();

  Config::SetDefault("ns3::McUePdcp::LteUplink", BooleanValue(lteUplink));
  std::cout << "Lte uplink " << lteUplink << "\n";

  /********** settings for the 3GPP the channel **********/
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ChannelCondition", StringValue("a"));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario", StringValue("UMi-StreetCanyon"));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::OptionalNlos", BooleanValue(true));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Shadowing", BooleanValue(true)); // enable or disable the shadowing effect
  Config::SetDefault ("ns3::MmWave3gppBuildingsPropagationLossModel::UpdateCondition", BooleanValue(true)); // enable or disable the LOS/NLOS update when the UE moves
  Config::SetDefault ("ns3::AntennaArrayModel::AntennaHorizontalSpacing", DoubleValue(0.5));
  Config::SetDefault ("ns3::AntennaArrayModel::AntennaVerticalSpacing", DoubleValue(0.5));
  Config::SetDefault ("ns3::MmWave3gppChannel::UpdatePeriod", TimeValue(MilliSeconds(100))); // interval after which the channel for a moving user is updated, 
                                                                                       // with spatial consistency procedure. If 0, spatial consistency is not used
  Config::SetDefault ("ns3::MmWave3gppChannel::CellScan", BooleanValue(false)); // Set true to use cell scanning method, false to use the default power method.
  Config::SetDefault ("ns3::MmWave3gppChannel::Blockage", BooleanValue(true)); // use blockage or not
  Config::SetDefault ("ns3::MmWave3gppChannel::PortraitMode", BooleanValue(true)); // use blockage model with UT in portrait mode
  Config::SetDefault ("ns3::MmWave3gppChannel::NumNonselfBlocking", IntegerValue(4)); // number of non-self blocking obstacles
  /********** settings for the 3GPP the channel **********/

  /********** settings building materials **********/
  Config::SetDefault ("ns3::Building::Type", StringValue("Residential"));
  Config::SetDefault ("ns3::Building::ExternalWallsType", StringValue("ConcreteWithWindows"));
  /********** settings building materials **********/

  /********** settings for the pathloss model **********/
  Ptr<MmWaveHelper> mmwaveHelper = CreateObject<MmWaveHelper> (); 
  if(true)
  {
    mmwaveHelper->SetAttribute ("PathlossModel", StringValue ("ns3::MmWave3gppBuildingsPropagationLossModel"));
  }
  else
  {
    mmwaveHelper->SetAttribute ("PathlossModel", StringValue ("ns3::MmWave3gppPropagationLossModel"));
    //  mmwaveHelper->SetAttribute ("PathlossModel", StringValue ("ns3::BuildingsObstaclePropagationLossModel"));
  }
  /********** settings for the pahloss model **********/

  mmwaveHelper->SetAttribute ("LteHandoverAlgorithm", StringValue ("ns3::A2A4RsrqHandoverAlgorithm"));

  mmwaveHelper->SetAttribute ("ChannelModel", StringValue ("ns3::MmWave3gppChannel"));

  //Ptr<MmWaveHelper> mmwaveHelper = CreateObject<MmWaveHelper> ();
  //mmwaveHelper->SetSchedulerType ("ns3::MmWaveFlexTtiMaxWeightMacScheduler");
  Ptr<MmWavePointToPointEpcHelper> epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
  mmwaveHelper->SetEpcHelper (epcHelper);
  mmwaveHelper->SetHarqEnabled (harqEnabled);

  mmwaveHelper->Initialize();

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  // parse again so you can override default values from the command line
  cmd.Parse(argc, argv);

   // Get SGW/PGW and create a single RemoteHost 
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  /********** Create the Internet by connecting remoteHost to pgw. Setup routing too **********/
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (2500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1); // interface 0 is localhost, 1 is the p2p device
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  /********** Create the Internet by connecting remoteHost to pgw. Setup routing too **********/

  /********** SIM TIME **********/
  double transientDuration = double(vectorTransient)/1000000;
  /********** SIM TIME **********/

  /********** create LTE, mmWave eNB nodes and UE node **********/
  NodeContainer lteEnbNodes;
  NodeContainer mmWaveEnbNodes;
  NodeContainer ueNodes;
  /********** create LTE, mmWave eNB nodes and UE node **********/

  /********** Create Buildings **********/

  /********** Set the height of the building with a U(15,25) **********/
  Ptr<UniformRandomVariable> zMax = CreateObject<UniformRandomVariable>();
  zMax->SetAttribute("Min",DoubleValue(15));
  zMax->SetAttribute("Max",DoubleValue(25));
  double zHeight = zMax->GetValue();
  double nFloors = round(zHeight/3);
  /********** Set the height of the building with a U(15,25) **********/

  /********** Declare Position Allocators for eNBs and UEs **********/
  Ptr<ListPositionAllocator> lteEnbPos = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> mmWaveEnbPos = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> uePos = CreateObject<ListPositionAllocator> ();
  /********** Declare Position Allocators for eNBs and UEs **********/

  switch (scenario)
  	{

  		case 1:
  		{
  			// Case 1: A general street scenario in a 160x80 map
  			// Reference: End-to-End Simulation of 5G mmWave Networks
  			netanim_scenario = "scenario-1-overview.xml";
  			gnuplot_scenario_buildings = "scenario-1-buildings.txt";
			gnuplot_scenario_enbs = "scenario-1-enbs.txt";
			gnuplot_scenario_ues = "scenario-1-ues.txt";

  			/********** network initializations**********/
  			lteEnbNodes.Create(1);
  			mmWaveEnbNodes.Create(2);
  			ueNodes.Create(1);
  			/********** network initializations**********/

  			/********** Set positions **********/
  			mmWaveEnbPos->Add(Vector(50, 65, 3));
  			mmWaveEnbPos->Add(Vector(150, 65, 3));
  			lteEnbPos->Add(Vector(100, 40, 3));
  			uePos->Add(Vector(50, 5, ueHeight));
  			// start UE movement after Seconds(0.5); Go in (+) x direction for 10-transientDuration seconds =~ 100m
			Simulator::Schedule(Seconds(transientDuration), &ChangeSpeed, ueNodes.Get(0), Vector(ueSpeed, 0, 0));
			simTime = 10;
  			/********** Set positions **********/

  			Ptr < Building > building1;
  			building1 = Create<Building> ();
  			building1->SetBoundaries (Box (55.0,60.0,
  										  15.0, 25.0,
  										  0.0, zHeight));
			building1->SetNRoomsX (nXrooms);
			building1->SetNRoomsY (nYrooms);
			building1->SetNFloors (nFloors);
  		    //building1->SetNFloors (1);

  			Ptr < Building > building2;
  			building2 = Create<Building> ();
  			building2->SetBoundaries (Box (80.0,95.0,
  			  							   20.0, 35.0,
  			  							   0.0, zHeight));
			building2->SetNRoomsX (nXrooms);
			building2->SetNRoomsY (nYrooms);
			building2->SetNFloors (nFloors);

  			Ptr < Building > building3;
  			building3 = Create<Building> ();
  			building3->SetBoundaries (Box (70.0,80.0,
  			  							   40.0, 45.0,
  			  							   0.0, zHeight));
			building3->SetNRoomsX (nXrooms);
			building3->SetNRoomsY (nYrooms);
			building3->SetNFloors (nFloors);

  			Ptr < Building > building4;
  			building4 = Create<Building> ();
  			building4->SetBoundaries (Box (105.0,110.0,
  			  							   35.0, 50.0,
  			  							   0.0, zHeight));
			building4->SetNRoomsX (nXrooms);
			building4->SetNRoomsY (nYrooms);
			building4->SetNFloors (nFloors);

  			Ptr < Building > building5;
  			building5 = Create<Building> ();
  			building5->SetBoundaries (Box (105.0,115.0,
  			  							   10.0, 20.0,
  			  							   0.0, zHeight));
			building5->SetNRoomsX (nXrooms);
			building5->SetNRoomsY (nYrooms);
			building5->SetNFloors (nFloors);

  			Ptr < Building > building6;
  			building6 = Create<Building> ();
  			building6->SetBoundaries (Box (130.0,145.0,
  			  							   15.0, 20.0,
  			  							   0.0, zHeight));
			building6->SetNRoomsX (nXrooms);
			building6->SetNRoomsY (nYrooms);
			building6->SetNFloors (nFloors);

  			Ptr < Building > building7;
  			building7 = Create<Building> ();
  			building7->SetBoundaries (Box (150.0,155.0,
  			  							   30.0, 35.0,
  			  							   0.0, zHeight));
			building7->SetNRoomsX (nXrooms);
			building7->SetNRoomsY (nYrooms);
			building7->SetNFloors (nFloors);

  			Ptr < Building > building8;
  			building8 = Create<Building> ();
  			building8->SetBoundaries (Box (135.0,155.0,
  			  							   25.0, 45.0,
  			  							   0.0, zHeight));
			building8->SetNRoomsX (nXrooms);
			building8->SetNRoomsY (nYrooms);
			building8->SetNFloors (nFloors);

  			break;
  		}
  		case 2:
  		{
  			// Case 2: A general street scenario in a 200x120 map
  			// Reference: Improved Handover through Dual COnnectivity in 5G mmWave Mobile Networks
  			netanim_scenario = "scenario-2-overview.xml";
  			gnuplot_scenario_buildings = "scenario-2-buildings.txt";
			gnuplot_scenario_enbs = "scenario-2-enbs.txt";
			gnuplot_scenario_ues = "scenario-2-ues.txt";

  			/********** network initializations**********/
  			lteEnbNodes.Create(1);
  			mmWaveEnbNodes.Create(2);
  			ueNodes.Create(1);
  			/********** network initializations**********/

  			/********** Set positions **********/
  			mmWaveEnbPos->Add(Vector(0, 50, 3));
  			mmWaveEnbPos->Add(Vector(100, 100, 3));
  			lteEnbPos->Add(Vector(100, 100, 3));
  			uePos->Add(Vector(0, 0, ueHeight));
			// start UE movement after Seconds(0.5); Go in (+) x direction for 10-transientDuration seconds =~ 100m
			Simulator::Schedule(Seconds(transientDuration), &ChangeSpeed, ueNodes.Get(0), Vector(ueSpeed, 0, 0));
			// Go in (+) y direction for 8 seconds = 80m
			Simulator::Schedule(Seconds(10), &ChangeSpeed, ueNodes.Get(0), Vector(0, ueSpeed, 0));
			// Go in (+) x direction for 9 seconds = 90m
			Simulator::Schedule(Seconds(18), &ChangeSpeed, ueNodes.Get(0), Vector(ueSpeed, 0, 0));
			// Go in (-) y direction for 8 seconds = 80m
			Simulator::Schedule(Seconds(27), &ChangeSpeed, ueNodes.Get(0), Vector(0, -ueSpeed, 0));
			simTime = 36;
  			/********** Set positions **********/

  			Ptr < Building > building1;
  			building1 = Create<Building> ();
  			building1->SetBoundaries (Box (40.0,50.0,
  										  80.0, 110.0,
  										  0.0, zHeight));
			building1->SetNRoomsX (nXrooms);
			building1->SetNRoomsY (nYrooms);
			building1->SetNFloors (nFloors);
  		    //building1->SetNFloors (1);

  			Ptr < Building > building2;
  			building2 = Create<Building> ();
  			building2->SetBoundaries (Box (50.0,125.0,
  			  							   60.0, 90.0,
  			  							   0.0, zHeight));
			building2->SetNRoomsX (nXrooms);
			building2->SetNRoomsY (nYrooms);
			building2->SetNFloors (nFloors);

  			Ptr < Building > building3;
  			building3 = Create<Building> ();
  			building3->SetBoundaries (Box (80.0,180.0,
  			  							   10.0, 40.0,
  			  							   0.0, zHeight));
			building3->SetNRoomsX (nXrooms);
			building3->SetNRoomsY (nYrooms);
			building3->SetNFloors (nFloors);

  			Ptr < Building > building4;
  			building4 = Create<Building> ();
  			building4->SetBoundaries (Box (160.0,180.0,
  			  							   60.0, 90.0,
  			  							   0.0, zHeight));
			building4->SetNRoomsX (nXrooms);
			building4->SetNRoomsY (nYrooms);
			building4->SetNFloors (nFloors);

  			break;
  		}
  		case 3:
		{
			// Case 3: One mmWave and one LTE with two sources of blockage
			// 	 	   simulating the street corridor (100x15 map)
			// Reference: Accuracy Comparison of Propagation Models for mmWave Communication in NS-3
  			netanim_scenario = "scenario-3-overview.xml";
  			gnuplot_scenario_buildings = "scenario-3-buildings.txt";
			gnuplot_scenario_enbs = "scenario-3-enbs.txt";
			gnuplot_scenario_ues = "scenario-3-ues.txt";

			/********** network initializations**********/
			lteEnbNodes.Create(1);
			ueNodes.Create(1);
			mmWaveEnbNodes.Create(1);
			/********** network initializations**********/

			/********** Set positions **********/
			mmWaveEnbPos->Add(Vector(50, 0, 3));
			lteEnbPos->Add(Vector(50, 0, 3));
			uePos->Add(Vector(0, 15, ueHeight));
			// start UE movement after Seconds(0.5); Go in (+) x direction for 10-transientDuration seconds =~ 100m
			Simulator::Schedule(Seconds(transientDuration), &ChangeSpeed, ueNodes.Get(0), Vector(ueSpeed, 0, 0));
			simTime = 10;
			/********** Set positions **********/

			Ptr < Building > building1;
			building1 = Create<Building> ();
			building1->SetBoundaries (Box (25.0,35.0,
										   5.0, 10.0,
										   0.0, zHeight));
			building1->SetNRoomsX (nXrooms);
			building1->SetNRoomsY (nYrooms);
			building1->SetNFloors (nFloors);
			//building1->SetNFloors (1);

			Ptr < Building > building2;
			building2 = Create<Building> ();
			building2->SetBoundaries (Box (65.0,75.0,
										   5.0, 10.0,
										   0.0, zHeight));
			building2->SetNRoomsX (nXrooms);
			building2->SetNRoomsY (nYrooms);
			building2->SetNFloors (nFloors);

			break;
		}
  		case 4:
		{
			// Case 4: One mmWave and one LTE with with one source of blockage simulation used
			// 		   to provide best possible overview of our enhancement. (100x15 map)
			// Reference: Accuracy Comparison of Propagation Models for mmWave Communication in NS-3
  			netanim_scenario = "scenario-4-overview.xml";
  			gnuplot_scenario_buildings = "scenario-4-buildings.txt";
			gnuplot_scenario_enbs = "scenario-4-enbs.txt";
			gnuplot_scenario_ues = "scenario-4-ues.txt";

			/********** network initializations**********/
			lteEnbNodes.Create(1);
			mmWaveEnbNodes.Create(1);
			ueNodes.Create(1);
			/********** network initializations**********/

			/********** Set positions **********/
			mmWaveEnbPos->Add(Vector(50, 0, 3));
			lteEnbPos->Add(Vector(50, 0, 3));
			uePos->Add(Vector(0, 15, ueHeight));
			// start UE movement after Seconds(0.5); Go in (+) x direction for 10-transientDuration seconds =~ 100m
			Simulator::Schedule(Seconds(transientDuration), &ChangeSpeed, ueNodes.Get(0), Vector(ueSpeed, 0, 0));
			simTime = 10;
			/********** Set positions **********/

			Ptr < Building > building1;
			building1 = Create<Building> ();
			building1->SetBoundaries (Box (40.0,60.0,
										  5.0, 10.0,
										  0.0, zHeight));
			building1->SetNRoomsX (1);
			building1->SetNRoomsY (1);
			building1->SetNFloors (1);

			break;
		}
  		case 5:
		{
			// Case 5: Two mmWaves and one LTE two sources of blockage simulating the
			//		   street corridor with multiple ENBs for better coverage
			// 	 	   and minimalisation of blind spots. (100x15 map)
			// Reference: Accuracy Comparison of Propagation Models for mmWave Communication in NS-3
  			netanim_scenario = "scenario-5-overview.xml";
  			gnuplot_scenario_buildings = "scenario-5-buildings.txt";
			gnuplot_scenario_enbs = "scenario-5-enbs.txt";
			gnuplot_scenario_ues = "scenario-5-ues.txt";

			/********** network initializations**********/
			lteEnbNodes.Create(1);
			ueNodes.Create(1);
			mmWaveEnbNodes.Create(2);
			/********** network initializations**********/

			/********** Set positions **********/
  			mmWaveEnbPos->Add(Vector(30, 0, 3));
  			mmWaveEnbPos->Add(Vector(70, 0, 3));
  			lteEnbPos->Add(Vector(50, 0, 3));
  			uePos->Add(Vector(0, 15, ueHeight));
			// start UE movement after Seconds(0.5); Go in (+) x direction for 10-transientDuration seconds =~ 100m
			Simulator::Schedule(Seconds(transientDuration), &ChangeSpeed, ueNodes.Get(0), Vector(ueSpeed, 0, 0));
			simTime = 10;
			/********** Set positions **********/

			Ptr < Building > building1;
			building1 = Create<Building> ();
			building1->SetBoundaries (Box (25.0,35.0,
										  5.0, 10.0,
										  0.0, zHeight));
			building1->SetNRoomsX (nXrooms);
			building1->SetNRoomsY (nYrooms);
			building1->SetNFloors (nFloors);
			//building1->SetNFloors (1);

			Ptr < Building > building2;
			building2 = Create<Building> ();
			building2->SetBoundaries (Box (65.0,75.0,
										   5.0, 10.0,
										   0.0, zHeight));
			building2->SetNRoomsX (nXrooms);
			building2->SetNRoomsY (nYrooms);
			building2->SetNFloors (nFloors);

			break;
		}
  		case 6:
		{
			// Case 6: Two mmWaves and one LTE one source of blockage. (100x15 map)
			// Reference: Accuracy Comparison of Propagation Models for mmWave Communication in NS-3
  			netanim_scenario = "scenario-6-overview.xml";
  			gnuplot_scenario_buildings = "scenario-6-buildings";
			gnuplot_scenario_enbs = "scenario-6-enbs.txt";
			gnuplot_scenario_ues = "scenario-6-ues.txt";

			/********** network initializations**********/
			lteEnbNodes.Create(1);
			mmWaveEnbNodes.Create(2);
			ueNodes.Create(1);
			/********** network initializations**********/

			/********** Set positions **********/
			mmWaveEnbPos->Add(Vector(35, 0, 3));
			mmWaveEnbPos->Add(Vector(65, 0, 3));
			lteEnbPos->Add(Vector(50, 0, 3));
			uePos->Add(Vector(0, 15, ueHeight));
			// start UE movement after Seconds(0.5); Go in (+) x direction for 10-transientDuration seconds =~ 100m
			Simulator::Schedule(Seconds(transientDuration), &ChangeSpeed, ueNodes.Get(0), Vector(ueSpeed, 0, 0));
			simTime = 10;
			/********** Set positions **********/

			Ptr < Building > building1;
			building1 = Create<Building> ();
			building1->SetBoundaries (Box (40.0,60.0,
										  5.0, 10.0,
										  0.0, zHeight));
			building1->SetNRoomsX (nXrooms);
			building1->SetNRoomsY (nYrooms);
			building1->SetNFloors (nFloors);
			//building1->SetNFloors (1);

			break;
		}
  		case 7:
		{
			// Case 7: Corner turn case
			// Reference: Improved Handover through Dual COnnectivity in 5G mmWave Mobile Networks
  			netanim_scenario = "scenario-7-overview.xml";
  			gnuplot_scenario_buildings = "scenario-7-buildings.txt";
			gnuplot_scenario_enbs = "scenario-7-enbs.txt";
			gnuplot_scenario_ues = "scenario-7-ues.txt";

			/********** network initializations**********/
			lteEnbNodes.Create(1);
			mmWaveEnbNodes.Create(3);
			ueNodes.Create(1);
			/********** network initializations**********/

			/********** Set positions **********/
			mmWaveEnbPos->Add(Vector(200, 10, 3));
			mmWaveEnbPos->Add(Vector(0, 10, 3));
			mmWaveEnbPos->Add(Vector(100, 100, 3));
			lteEnbPos->Add(Vector(100, 100, 3));
			uePos->Add(Vector(0, 0, ueHeight));
			// start UE movement after Seconds(0.5); Go in (+) x direction for 10-transientDuration seconds =~ 100m
			Simulator::Schedule(Seconds(transientDuration), &ChangeSpeed, ueNodes.Get(0), Vector(ueSpeed, 0, 0));
			// Go in (+) y direction for 9 seconds = 90m
			Simulator::Schedule(Seconds(10), &ChangeSpeed, ueNodes.Get(0), Vector(0, ueSpeed, 0));
			// Go in (-) y direction for 9 seconds = 90m
			Simulator::Schedule(Seconds(19), &ChangeSpeed, ueNodes.Get(0), Vector(0, -ueSpeed, 0));
			// Go in (+) x direction for 10 seconds = 100m
			Simulator::Schedule(Seconds(28), &ChangeSpeed, ueNodes.Get(0), Vector(ueSpeed, 0, 0));
			simTime = 38;
			/********** Set positions **********/

			Ptr < Building > building1;
			building1 = Create<Building> ();
			building1->SetBoundaries (Box (5.0,95.0,
										  20.0, 130.0,
										  0.0, zHeight));
			building1->SetNRoomsX (nXrooms);
			building1->SetNRoomsY (nYrooms);
			building1->SetNFloors (nFloors);

			Ptr < Building > building2;
			building2 = Create<Building> ();
			building2->SetBoundaries (Box (105.0,195.0,
										  20.0, 130.0,
										  0.0, zHeight));
			building2->SetNRoomsX (nXrooms);
			building2->SetNRoomsY (nYrooms);
			building2->SetNFloors (nFloors);

			//building1->SetNFloors (1);

			break;
		}
  		default:
  		{
  		    NS_FATAL_ERROR ("Invalid scenario");
  		}
  	}
  /********** Create Buildings **********/

  /********** Install Mobility Models and Initial positions **********/
  MobilityHelper lteEnbMobility;
  lteEnbMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  lteEnbMobility.SetPositionAllocator(lteEnbPos);
  lteEnbMobility.Install (lteEnbNodes);
  BuildingsHelper::Install (lteEnbNodes);

  MobilityHelper mmWaveEnbMobility;
  mmWaveEnbMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mmWaveEnbMobility.SetPositionAllocator(mmWaveEnbPos);
  mmWaveEnbMobility.Install (mmWaveEnbNodes);
  BuildingsHelper::Install (mmWaveEnbNodes);

  MobilityHelper uemobility;
  uemobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  uemobility.SetPositionAllocator(uePos);
  uemobility.Install (ueNodes);
  BuildingsHelper::Install (ueNodes);
  ueNodes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (0, 0, 0));
  /********** Install Mobility Models and Initial positions **********/

  // Install mmWave, lte, mc Devices to the nodes
  NetDeviceContainer lteEnbDevs = mmwaveHelper->InstallLteEnbDevice (lteEnbNodes);
  NetDeviceContainer mmWaveEnbDevs = mmwaveHelper->InstallEnbDevice (mmWaveEnbNodes);
  NetDeviceContainer mcUeDevs;
  if(fastSwitching)
  {
    mcUeDevs = mmwaveHelper->InstallMcUeDevice (ueNodes);
  } 
  else if(hardHandover)
  {
    mcUeDevs = mmwaveHelper->InstallInterRatHoCapableUeDevice (ueNodes);
  }
  else
  {
    NS_FATAL_ERROR("Invalid option");
  }

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (mcUeDevs));
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
  {
    Ptr<Node> ueNode = ueNodes.Get (u);
    // Set the default gateway for the UE
    Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
    ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
  }

  // Add X2 interfaces
  mmwaveHelper->AddX2Interface (lteEnbNodes, mmWaveEnbNodes);

  //

  //

  // Manual attachment
  if(fastSwitching)
  {
	/********** Attach MC ueDevices to the closest LTE enbDevice, register all MmWave eNBs to the MmWaveUePhy **********/
    mmwaveHelper->AttachToClosestEnb (mcUeDevs, mmWaveEnbDevs, lteEnbDevs);  
  }
  else if(hardHandover)
  {
    mmwaveHelper->AttachIrToClosestEnb (mcUeDevs, mmWaveEnbDevs, lteEnbDevs);
  }
  
  // Install and start applications on UEs and remote host
  uint16_t dlPort = 1.234;
  uint16_t ulPort = 2.000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  bool dl = 0;
  bool ul = 0;

  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
  {
      if(dl)
      {
        UdpServerHelper dlPacketSinkHelper (dlPort);
        dlPacketSinkHelper.SetAttribute ("PacketWindowSize", UintegerValue(256));
        serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get(u)));

        // Simulator::Schedule(MilliSeconds(20), &PrintLostUdpPackets, DynamicCast<UdpServer>(serverApps.Get(serverApps.GetN()-1)), lostFilename);

        UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
        dlClient.SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
        dlClient.SetAttribute ("MaxPackets", UintegerValue(0xFFFFFFFF));
        clientApps.Add (dlClient.Install (remoteHost));

      }
      if(ul)
      {
        ++ulPort;
        PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
        ulPacketSinkHelper.SetAttribute ("PacketWindowSize", UintegerValue(256));
        serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
        UdpClientHelper ulClient (remoteHostAddr, ulPort);
        ulClient.SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
        ulClient.SetAttribute ("MaxPackets", UintegerValue(0xFFFFFFFF));
        clientApps.Add (ulClient.Install (ueNodes.Get(u)));
      }
  }



  // Start applications
  NS_LOG_UNCOND("transientDuration " << transientDuration << " simTime " << simTime);
  serverApps.Start (Seconds(transientDuration));
  clientApps.Start (Seconds(transientDuration));
  clientApps.Stop (Seconds(simTime - 1));


  // stop UE movement after Seconds(0.5)
  Simulator::Schedule(Seconds(simTime + 0.5), &ChangeSpeed, ueNodes.Get(0), Vector(0, 0, 0));

  double numPrints = 10;
  for(int i = 0; i < numPrints; i++)
  {
   Simulator::Schedule(Seconds(i*simTime/numPrints), &PrintPosition, ueNodes.Get(0)); 
  }

  BuildingsHelper::MakeMobilityModelConsistent ();

  mmwaveHelper->EnableTraces ();

  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/ConnectionEstablished",
                     MakeCallback (&NotifyConnectionEstablishedEnb));
    Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/ConnectionEstablished",
                     MakeCallback (&NotifyConnectionEstablishedUe));
    Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverStart",
                     MakeCallback (&NotifyHandoverStartEnb));
    Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/HandoverStart",
                     MakeCallback (&NotifyHandoverStartUe));
    Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverEndOk",
                     MakeCallback (&NotifyHandoverEndOkEnb));
    Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/HandoverEndOk",
                     MakeCallback (&NotifyHandoverEndOkUe));

  // set to print = True if you want to print the map of buildings, ues and enbs
  if(print)
  {
    PrintGnuplottableBuildingListToFile(gnuplot_scenario_buildings);
    PrintGnuplottableEnbListToFile(gnuplot_scenario_enbs);
    PrintGnuplottableUeListToFile(gnuplot_scenario_ues);
  }
  
  Simulator::Stop(Seconds(simTime));
  AnimationInterface anim(netanim_scenario);
  Simulator::Run();

  Simulator::Destroy();
  return 0;
}

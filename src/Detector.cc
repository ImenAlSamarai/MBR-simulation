#include "Detector.h"
#include "DetectorSetup.h"
#include "Configuration.h"
#include <ChannelMap.h>
#include <EventFile.h>
#include <PhysicalConstants.h>
#include <MathConstants.h>
#include <stdexcept>
#include <iostream>
#include <string>
#include <cmath>
#include <tinyxml.h>
#include <tinyxmlCromeUtilities.h>


using namespace std;
using namespace utl;

Detector::Detector()
  : fDetectorSetup(NULL)
{

}


void Detector::InitFromFile(EventFile& file)
{

  fDetectorSetup = new DetectorSetup();
  file.ReadDetectorSetup(*fDetectorSetup);

}

void Detector::Init()
{
  fVerbosity = Configuration::GetInstance().GetVerbosity();

  fDetectorSetup = new DetectorSetup();

  if ( fVerbosity > 0 )
    cout << "\n  ==[Detector::Init()]==>" << endl;

  const string configId("MicrowaveDetector");
  const string& fileName =
    Configuration::GetInstance().GetConfigurationFileName(configId);

  TiXmlDocument doc(fileName);
  if ( doc.LoadFile() ) {

    const TiXmlElement* topBranch = doc.FirstChildElement(configId);
    if (!topBranch ) {
      throw std::runtime_error(string("Detector::Init(): ")+
                               string("could not find MicrowaveDetector in ")+
                               fileName + ".");
    }

    // initialize array
    const TiXmlElement* arrayBranch = topBranch->FirstChildElement("array");
    if (!arrayBranch )
      throw std::runtime_error(string("Detector::Init(): ")+
                               string("could not find array branch in ")+
                               fileName);

    if ( !InitArray(arrayBranch->Attribute("type")) )
      throw std::runtime_error(string("Detector::Init(): ")+
                               string("error reading array config"));

    // initialize antenna(s)
    const TiXmlElement* antennaBranch = topBranch->FirstChildElement("antenna");
    while( antennaBranch ) {
      if ( !AddAntenna(antennaBranch) )
        throw std::runtime_error(string("Detector::Init(): ")+
                                 string("error reading antenna config"));

      antennaBranch = antennaBranch->NextSiblingElement("antenna");
    }
  }
  else {
    throw std::runtime_error(string("Detector::Init(): ")+
                             string("error opening/reading ")+
                             fileName);
  }

}


void Detector::DeInit()
{

  delete fDetectorSetup;
  fDetectorSetup = NULL;

}


Detector::~Detector() {

  delete fDetectorSetup;

}


bool Detector::AddAntenna(const TiXmlElement* antennaBranch) {

  Antenna antenna;

  int antennaId;
  if ( antennaBranch->QueryIntAttribute("id",&antennaId) ==
       TIXML_SUCCESS ) {

    antenna.SetId(antennaId);

    const double elevation =
      GetDoubleValue(GetFirstChild(antennaBranch, "elevation"));
    antenna.SetElevation(elevation * degree);

    const double azimuth =
      GetDoubleValue(GetFirstChild(antennaBranch, "azimuth"));
    antenna.SetAzimuth(azimuth * degree);

    const double cameraAzimuth =
      GetDoubleValue(GetFirstChild(antennaBranch, "cameraAzimuth"));
    antenna.SetCameraAzimuth(cameraAzimuth * degree);

    antenna.SetPosition(GetPosition(antennaBranch));

    const TiXmlElement* dishBranch = antennaBranch->FirstChildElement("dish");
    if ( !dishBranch )
      return false;
    const char* dishType = dishBranch->Attribute("type");

    if (!SetDishProperties(dishType, antenna) )
      return false;

    const TiXmlElement* channelBranch = GetFirstChild(antennaBranch, "channel");
    while ( channelBranch ) {
      AddChannel(channelBranch, antenna);
      channelBranch = channelBranch->NextSiblingElement("channel");
    }

    const double distanceToFarField =
      GetDoubleValue(GetFirstChild(antennaBranch, "distanceToFarField"));
    antenna.SetDistanceToFarField(distanceToFarField * m);

    if ( fVerbosity > 0 ) {
      cout << "   antenna number " << antennaId << endl
           << "         elevation: " << elevation << endl
           << "         azimuth: " << azimuth << endl
           << "         cameraAzimuth: " << cameraAzimuth << endl
           << "         nChannel: " << antenna.GetNumberOfChannels() << endl
           << "         dishType: " << dishType << endl;
    }

    fDetectorSetup->AddAntenna(antenna);

    return true;
  }

  return false;

}

void Detector::AddChannel(const TiXmlElement* branch, Antenna& antenna) {

  Channel channel(antenna.GetId());
  channel.SetId(GetUnsignedAttribute(branch, "id"));

  const double elevation =
    GetDoubleValue(GetFirstChild(branch, "elevation"));
  channel.SetLocalElevation(elevation*degree);

  const double azimuth =
    GetDoubleValue(GetFirstChild(branch, "azimuth"));
  channel.SetLocalAzimuth(azimuth*degree);

  const double cableDelay =
    GetDoubleValue(GetFirstChild(branch, "cableDelay"));
  channel.SetCableDelay(cableDelay * ns);

  channel.SetLocalPosition(GetPosition(branch));

  SetChannelProperties(GetStringAttribute(branch, "type"), channel);

  double beamwidth = 0 * degree;
  if ( HasBranch(branch, "beamwidth") ) {
    // get beamwidth from XML
    beamwidth = GetDoubleValue(GetFirstChild(branch, "beamwidth")) * degree;
  } else {
    // calculate approximate angular beam width as in Fig. 1.9 in
    // R.C. Johnson - Antenna Engineering Handbook
    const double diameter = antenna.GetDiameter();
    const double wavelength = kSpeedOfLight / channel.GetCenterFrequency();
    beamwidth = 70 * degree * wavelength / diameter;
  }
  channel.SetBeamWidth(beamwidth);

  TVector3 direction(0, 0, 1);
  channel.RotateFromChannelToAntenna(direction);
  antenna.RotateFromAntennaToArray(direction);
  channel.SetPointingDirection(direction);
  channel.SetGlobalPosition(antenna.GetPosition());

  // get gain or gain pattern file
  if ( HasBranch(branch, "gainPatternFile") ) {
    if ( HasBranch(branch, "gain") ) {
      throw runtime_error("Detector::AddChannel(): "
                          "Channel has specified gain and gainPatternFile. "
                          "Only one can be used.");
    }

    const string gainFilename =
      GetStringValue(GetFirstChild(branch, "gainPatternFile"));
    channel.MakeResponse(gainFilename, antenna);
  } else {
    // Read gain (stored in XML in dBi) and store it as factor
    const double boresightGain = GetDoubleValue(GetFirstChild(branch, "gain"));
    channel.SetBoresightGain(pow(10., boresightGain / 10.));
  }

  antenna.AddChannel(channel);

}


bool Detector::InitArray(const char* arrayType) {

  if ( !arrayType )
    return false;

  if ( fVerbosity > 0 )
    cout << "   array type: " << arrayType << endl;

  Array& array = fDetectorSetup->GetArray();

  const string configId("ArrayModels");
  const string& fileName =
    Configuration::GetInstance().GetConfigurationFileName(configId);

  TiXmlDocument doc(fileName);
  if ( !doc.LoadFile() ) {
    return false;
  }

  const TiXmlElement* topBranch = doc.FirstChildElement(configId);
  if ( !topBranch ) {
    return false;
  }

  bool foundArrayType = false;
  const TiXmlElement* arrayBranch = topBranch->FirstChildElement("array");
  while( arrayBranch ) {
    if ( string(arrayBranch->Attribute("type")) == string(arrayType) ) {

      const double height =
        GetDoubleValue(GetFirstChild(arrayBranch, "height"));
      array.SetHeight(height*m);

      const double phi =
        GetDoubleValue(GetFirstChild(arrayBranch, "phi"));
      array.SetPhi(phi*degree);

      const double detectorDelay =
        GetDoubleValue(GetFirstChild(arrayBranch, "detectorDelay"));
      const double electronicsDelay =
        GetDoubleValue(GetFirstChild(arrayBranch, "electronicsDelay"));
      const double cableDelay =
        GetDoubleValue(GetFirstChild(arrayBranch, "cableDelay"));
      const double totalDelay =
        detectorDelay + electronicsDelay + cableDelay;
      array.SetSignalTimeDelay(totalDelay*nanosecond);

      const TiXmlElement* stationListBranch =
        arrayBranch->FirstChildElement("stationList");
      if ( !stationListBranch )
        return false;

      const TiXmlElement* stationBranch =
        stationListBranch->FirstChildElement("station");
      while( stationBranch ) {
        Station station;

        station.SetId(GetUnsignedAttribute(stationBranch, "id"));
        station.SetPosition(GetPosition(stationBranch));

        array.AddStation(station);

        stationBranch = stationBranch->NextSiblingElement("station");
      }

      if ( fVerbosity > 0 )
        cout << "         height=" << array.GetHeight() << "\n"
             << "         nStations= "
             << array.GetNumberOfStations() << endl;

      foundArrayType = true;
      break;
    }

    arrayBranch = arrayBranch->NextSiblingElement("array");
  }

  if ( !foundArrayType ) {
    return false;
  }

  // TODO: The rest of this function loads data from a ChannelMap into
  //       the Array. However, the ChannelMap should not contain any
  //       information about the array at all, only references to
  //       it. The information we need, namely the trigger time
  //       ranges, should be read from a separate XML file.

  // Look for a channel map
  try {
    Configuration::GetInstance().GetConfigurationFileName("ChannelMap");
  } catch (...) {
    // There's no channel map, so nothing left to do
    return true;
  }

  ChannelMap channelMap;

  // TODO: The interface of ChannelMap could be extended to allow
  //       iteration of station trigger time ranges. But: see above
  //       note.
  for ( Array::ConstStationIterator iter = array.StationsBegin();
        iter != array.StationsEnd(); ++iter ) {
    unsigned int stationId = iter->first;
    if (channelMap.HasStationTriggerTimeRange(stationId)) {
      array.GetStation(stationId).SetTriggerTimeRange(
        channelMap.GetStationTriggerTimeRange(stationId));
    }

    if (channelMap.HasStationTriggerTimeOffset(stationId)) {
      array.GetStation(stationId).SetTriggerTimeOffset(
        channelMap.GetStationTriggerTimeOffset(stationId));
    }
  }

  return true;
}


bool Detector::SetDishProperties(const char* dishType, Antenna& antenna) {

  if ( !dishType )
    return false;

  const string configId("DishModels");
  const string& fileName =
    Configuration::GetInstance().GetConfigurationFileName(configId);

  TiXmlDocument doc(fileName);
  if ( doc.LoadFile() ) {
    const TiXmlElement* topBranch = doc.FirstChildElement(configId);
    if ( topBranch ) {
      const TiXmlElement* dishBranch = topBranch->FirstChildElement("dish");
      while( dishBranch ) {
        if ( string(dishBranch->Attribute("type")) == string(dishType) ) {

          const double diameter =
            GetDoubleValue(GetFirstChild(dishBranch, "diameter"));
          antenna.SetDiameter(diameter * m);

          return true;
        }
        dishBranch = dishBranch->NextSiblingElement("dish");
      }
    }
  }

  return false;

}

void Detector::SetChannelProperties(string channelType, Channel& channel) {

  const string configId("ChannelModels");
  const string& filename =
    Configuration::GetInstance().GetConfigurationFileName(configId);

  TiXmlDocument doc(filename);
  if ( doc.LoadFile() ) {
    const TiXmlElement* topBranch = doc.FirstChildElement(configId);
    if ( topBranch ) {
      const TiXmlElement* channelBranch =
        topBranch->FirstChildElement("channel");
      while( channelBranch ) {
        if ( string(channelBranch->Attribute("type")) == channelType ) {
          const double traceBinWidth =
            GetDoubleValue(GetFirstChild(channelBranch, "traceBinWidth"));
          channel.SetTraceBinWidth(traceBinWidth * s);

          const double traceStartTime =
            GetDoubleValue(GetFirstChild(channelBranch, "traceStartTime"));
          channel.SetTraceStartTime(traceStartTime * s);

          const double bandWidth =
            GetDoubleValue(GetFirstChild(channelBranch, "bandWidth"));
          channel.SetBandWidth(bandWidth * hertz);

          const double centerFrequency =
            GetDoubleValue(GetFirstChild(channelBranch, "centerFrequency"));
          channel.SetCenterFrequency(centerFrequency * hertz);

          const double noise =
            GetDoubleValue(GetFirstChild(channelBranch, "noise"));
          channel.SetNoise(noise * kelvin);

          const unsigned int nBins =
            GetUnsignedValue(GetFirstChild(channelBranch, "nTraceBins"));
          channel.SetNTraceBins(nBins);

          return;
        }
        channelBranch = channelBranch->NextSiblingElement("channel");
      }
    }
  }

  throw runtime_error("SetChannelProperties(): Unknown channel type " +
                      channelType + ".");
}

#include "ChannelMap.h"

#include <boost/foreach.hpp>

#include <AugerUnits.h>
#include <tinyxml.h>
#include <tinyxmlCromeUtilities.h>

using namespace std;


ChannelMap::ChannelMap()
{
  // Get the filename of the channel map from the global configuration
  const string configId("ChannelMap");
  const string& filename =
    Configuration::GetInstance().GetConfigurationFileName(configId);

  // Parse the XML
  TiXmlDocument document(filename);
  if (!document.LoadFile()) {
    throw std::runtime_error(string("ChannelMap::Init(): Could not parse ") +
                             filename);
  }

  // Get the root node
  TiXmlElement* rootNode = document.FirstChildElement(configId.c_str());
  if (!rootNode) {
    throw std::runtime_error(string("ChannelMap::Init(): Could not find "
                                    "ChannelMap node in ") + filename);
  }

  // Loop over each channel
  if (Configuration::GetInstance().GetVerbosity() > 0) {
    cout << endl << "  ==[ChannelMap::Init()]==>" << endl;
  }

  TiXmlElement* channel = rootNode->FirstChildElement("channel");
  while (channel) {
    // Get the ids of digitizer and channel
    TiXmlElement* digitizer = GetFirstChild(channel, "digitizer");
    unsigned int digitizerId = GetUnsignedAttribute(digitizer, "id");

    TiXmlElement* digitizerChannel = GetFirstChild(digitizer, "channel");
    unsigned int digitizerChannelId = GetUnsignedValue(digitizerChannel);

#warning TODO Check whether all ids are available in detector setup
    if (Configuration::GetInstance().GetVerbosity() > 0) {
      cout << "   digitizer " << digitizerId;
      cout << ", channel " << digitizerChannelId;
    }

    // Create the key for our map and check whether it already exists
    pair<unsigned int, unsigned int> key(digitizerId, digitizerChannelId);
    if (sourceTypeMap.count(key) != 0) {
      stringstream err_msg_ss;
      err_msg_ss << "ChannelMap::Init(): Tried to add same digitizer channel";
      err_msg_ss << "twice (digitizer " << digitizerId << ", channel ";
      err_msg_ss << digitizerChannelId << ").";
      string err_msg = err_msg_ss.str();
      throw runtime_error(err_msg);
    }

    // Get the signal source
    TiXmlElement* signalSource = GetFirstChild(channel, "signalSource");
    string signalSourceType = GetStringAttribute(signalSource, "type");

    if (signalSourceType.compare("antenna") == 0) {
      // The signal of an LNB is connected to this channel
      // Get the ids of antenna, receiver, and electronics chain
      TiXmlElement* antenna = GetFirstChild(signalSource, "antenna");
      unsigned int antennaId = GetUnsignedAttribute(antenna, "id");
      unsigned int channelId = GetUnsignedAttribute(antenna, "channel");

      TiXmlElement* receiver = GetFirstChild(signalSource, "receiver");
      unsigned int receiverId = GetUnsignedAttribute(receiver, "id");

      TiXmlElement* electronicsChain = GetFirstChild(signalSource,
                                                     "electronicsChain");
      unsigned int electronicsChainId =
        GetUnsignedAttribute(electronicsChain, "id");

#warning TODO Check whether all the ids are available in detector setup
      if (Configuration::GetInstance().GetVerbosity() > 0) {
        cout << " <-> antenna " << antennaId << ", channel " << channelId;
        cout << " - receiver " << receiverId;
        cout << " - electronics chain " << electronicsChainId << endl;
      }

      // Create the value and check whether it already exists
      pair<unsigned int, unsigned int> value(antennaId, channelId);
      pair< pair<unsigned int, unsigned int>,
            pair<unsigned int, unsigned int> > item;
      BOOST_FOREACH(item, antennaMap) {
        if (item.second == value) {
          stringstream err_msg_ss;
          err_msg_ss << "ChannelMap::Init(): Tried to add same antenna channel";
          err_msg_ss << "twice (antenna " << antennaId << ", channel ";
          err_msg_ss << channelId << ").";
          string err_msg = err_msg_ss.str();
          throw runtime_error(err_msg);
        }
      }

      // Fill the maps
      sourceTypeMap[key] = eAntenna;
      antennaMap[key] = value;
    } else if (signalSourceType.compare("kgStation") == 0) {
      // The signal of a Grande station is connected to this channel
      // Get the id of the Grande station and the trigger time window
      TiXmlElement* kgStation = GetFirstChild(signalSource, "kgStation");
      unsigned int kgStationId = GetUnsignedAttribute(kgStation, "id");
      double kgStationMinTriggerTime =
        GetDoubleValue(GetFirstChild(kgStation, "minTriggerTime")) * utl::s;
      double kgStationMaxTriggerTime =
        GetDoubleValue(GetFirstChild(kgStation, "maxTriggerTime")) * utl::s;
      double kgStationTriggerTimeOffset =
        GetDoubleValue(GetFirstChild(kgStation, "triggerTimeOffset")) * utl::s;

#warning TODO Check whether the id is available in the array
      if (Configuration::GetInstance().GetVerbosity() > 0) {
        cout << " <-> KG station " << kgStationId << endl;
      }

      // Check whether it already exists
      pair<pair<unsigned int, unsigned int>, unsigned int> item;
      BOOST_FOREACH(item, stationMap) {
        if (item.second == kgStationId) {
          stringstream err_msg_ss;
          err_msg_ss << "ChannelMap::Init(): Tried to add same KG station";
          err_msg_ss << "twice (station id " << kgStationId << ").";
          string err_msg = err_msg_ss.str();
          throw runtime_error(err_msg);
        }
      }

      // Check for valid trigger window
      if (kgStationMaxTriggerTime <= kgStationMinTriggerTime) {
          stringstream err_msg_ss;
          err_msg_ss << "ChannelMap::Init(): minTriggerTime must be smaller ";
          err_msg_ss << "than maxTriggerTime (station id " << kgStationId;
          err_msg_ss << ").";
          string err_msg = err_msg_ss.str();
          throw runtime_error(err_msg);
      }

      // Fill the maps
      sourceTypeMap[key] = eKGStation;
      stationMap[key] = kgStationId;
      stationTriggerTimeRangeMap[kgStationId] =
        pair<double, double>(kgStationMinTriggerTime,
                             kgStationMaxTriggerTime);
      stationTriggerTimeOffsetMap[kgStationId] = kgStationTriggerTimeOffset;
    } else {
      throw std::runtime_error(string("ChannelMap::Init(): Type of signal "
                                      "source is unknown."));
    }

    channel = channel->NextSiblingElement("channel");
  }
}


// Returns the type of the connected signal source
eSignalSourceType
ChannelMap::GetSignalSourceType(const unsigned int& digitizerId,
                                const unsigned int& channelId)
{
  pair<unsigned int, unsigned int> key(digitizerId, channelId);
  if (sourceTypeMap.count(key) == 0) {
    stringstream err_msg_ss;
    err_msg_ss << "ChannelMap::GetSignalSourceType(): No entry for digitizer ";
    err_msg_ss << digitizerId << ", channel " << channelId << ".";
    string err_msg = err_msg_ss.str();
    throw runtime_error(err_msg);
  }

  return sourceTypeMap[key];
}


// Returns the ids of antenna and antenna channel connected to a digitizer channel
pair<unsigned int, unsigned int>
ChannelMap::GetAntennaChannelId(const unsigned int& digitizerId,
                                const unsigned int& channelId)
{
  pair<unsigned int, unsigned int> key(digitizerId, channelId);
  if (antennaMap.count(key) == 0) {
    stringstream err_msg_ss;
    err_msg_ss << "ChannelMap::GetAntennaChannelId(): No entry for digitizer ";
    err_msg_ss << digitizerId << ", channel " << channelId << ".";
    string err_msg = err_msg_ss.str();
    throw runtime_error(err_msg);
  }

  return antennaMap[key];
}


// Returns the id of the KASCADE Grande station connected to a digitizer channel
unsigned int ChannelMap::GetStationId(const unsigned int& digitizerId,
                                      const unsigned int& channelId)
{
  pair<unsigned int, unsigned int> key(digitizerId, channelId);
  if (stationMap.count(key) == 0) {
    stringstream err_msg_ss;
    err_msg_ss << "ChannelMap::GetStationId(): No entry for digitizer ";
    err_msg_ss << digitizerId << ", channel " << channelId << ".";
    string err_msg = err_msg_ss.str();
    throw runtime_error(err_msg);
  }

  return stationMap[key];
}


// Returns the expected trigger time window for the given KASCADE Grande
// station
pair<double, double>
  ChannelMap::GetStationTriggerTimeRange(const unsigned int& stationId)
{
  if (stationTriggerTimeRangeMap.count(stationId) == 0) {
    stringstream err_msg_ss;
    err_msg_ss << "ChannelMap::GetStationTriggerTimeRange(): No entry for ";
    err_msg_ss << "station " << stationId << ".";
    string err_msg = err_msg_ss.str();
    throw runtime_error(err_msg);
  }

  return stationTriggerTimeRangeMap[stationId];
}


// Returns the offset between the trigger condition and the rise of the
// signal
double ChannelMap::GetStationTriggerTimeOffset(const unsigned int& stationId)
{
  if (stationTriggerTimeOffsetMap.count(stationId) == 0) {
    stringstream err_msg_ss;
    err_msg_ss << "ChannelMap::GetStationTriggerTimeOffset(): No entry for ";
    err_msg_ss << "station " << stationId << ".";
    string err_msg = err_msg_ss.str();
    throw runtime_error(err_msg);
  }

  return stationTriggerTimeOffsetMap[stationId];
}

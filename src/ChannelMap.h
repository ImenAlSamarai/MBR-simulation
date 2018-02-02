#ifndef __ChannelMap__
#define __ChannelMap__

#include <string>
#include <map>
#include <utility>

#include "Configuration.h"


enum eSignalSourceType {
  eKGStation,
  eAntenna
};


class ChannelMap {
public:
  ChannelMap(void);

  // Returns the type of the connected signal source
  eSignalSourceType GetSignalSourceType(const unsigned int& digitizerId,
                                        const unsigned int& channelId);

  // Returns the ids of antenna and antenna channel connected to a digitizer
  // channel
  std::pair<unsigned int, unsigned int>
    GetAntennaChannelId(const unsigned int& digitizerId,
                        const unsigned int& channelId);

  // Returns the id of the KASCADE Grande station connected to a digitizer
  // channel
  unsigned int GetStationId(const unsigned int& digitizerId,
                            const unsigned int& channelId);

  // Returns the expected trigger time window for the given KASCADE Grande
  // station
  std::pair<double, double>
    GetStationTriggerTimeRange(const unsigned int& stationId);

  bool HasStationTriggerTimeRange(const unsigned int& stationId) {
    return stationTriggerTimeRangeMap.count(stationId) > 0;
  }

  // Returns the offset between the trigger condition and the rise of the
  // signal
  double GetStationTriggerTimeOffset(const unsigned int& stationId);

  bool HasStationTriggerTimeOffset(const unsigned int& stationId) {
    return stationTriggerTimeOffsetMap.count(stationId) > 0;
  }


private:
  std::map< std::pair<unsigned int, unsigned int>, eSignalSourceType>
    sourceTypeMap;

  std::map< std::pair<unsigned int, unsigned int>, unsigned int> stationMap;

  std::map< std::pair<unsigned int, unsigned int>,
    std::pair<unsigned int, unsigned int> > antennaMap;

  std::map<unsigned int, std::pair<double, double> > stationTriggerTimeRangeMap;

  std::map<unsigned int, double> stationTriggerTimeOffsetMap;
};

#endif

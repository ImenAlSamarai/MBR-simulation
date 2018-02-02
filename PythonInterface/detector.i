%module(package="mwoffline") detector

%include "common.i"

%{
#include <Singleton.h>

#include <Antenna.h>
#include <Array.h>
#include <Atmosphere.h>
#include <Channel.h>
#include <ChannelGain.h>
#include <ChannelMap.h>
#include <Detector.h>
#include <DetectorSetup.h>
#include <Station.h>
%}

// Instantiate templates so that they will be wrapped correctly
%template(AntennaVector) std::vector<Antenna>;
%template(ChannelVector) std::vector<Channel>;
%template(StationMap) std::map<int, Station>;
%import <Singleton.h>
%template(AtmosphereSingleton) utl::Singleton<Atmosphere>;
%template(DetectorSingleton) utl::Singleton<Detector>;

%include <Antenna.h>
%include <Array.h>
%include <Atmosphere.h>
%include <Channel.h>
%include <ChannelGain.h>
%include <ChannelMap.h>
%include <Detector.h>
%include <DetectorSetup.h>
%include <Station.h>

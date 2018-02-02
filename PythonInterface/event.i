%module(package="mwoffline") event

%include "common.i"
%import "detector.i"

%{
#include <Event.h>
#include <EventFile.h>
#include <ArrayData.h>
#include <SimData.h>
#include <Antenna.h>
#include <AntennaData.h>
#include <ChannelData.h>
#include <Channel.h>
#include <DetectorSetup.h>
#include <Shower.h>
#include <KGBits.h>
%}

// Wrap (compiler generated) copy constructors
%copyctor Shower;

%include <AntennaData.h>
%include <ArrayData.h>
%include <ChannelData.h>
%include <Event.h>
%include <EventFile.h>
%include <KGBits.h>
%include <Shower.h>
%include <SimData.h>
%include <StationData.h>

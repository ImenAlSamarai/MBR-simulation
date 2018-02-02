%module(package="mwoffline") simulation

%include "common.i"

%{
#include <GHShowerGenerator.h>
#include <GHShowerParameterization.h>
#include <TraceSimulator.h>
#include <BackgroundSimulator.h>
#include <ElectronicsSimulator.h>
#include <KGSimulator.h>
%}

%import(module="utilities") <TimedClass.h>

%include <GHShowerGenerator.h>
%include <GHShowerParameterization.h>
%include <TraceSimulator.h>
%include <BackgroundSimulator.h>
%include <ElectronicsSimulator.h>
%include <KGSimulator.h>

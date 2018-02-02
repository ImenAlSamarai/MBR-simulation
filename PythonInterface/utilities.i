%module(package="mwoffline") utilities

%include "common.i"

%{
#include <AugerUnits.h>
#include <BitTools.h>
#include <Configuration.h>
#include <KascadeEnergyConverter.h>
#include <LinearInterpolator.h>
#include <MathConstants.h>
#include <MathFunctions.h>
#include <PhysicalConstants.h>
#include <PhysicsFunctions.h>
#include <RandomGenerator.h>
#include <Singleton.h>
#include <TimeStamp.h>
#include <TimedClass.h>
%}

// Instantiate templates so that they will be wrapped correctly
%import <Singleton.h>
%template(ConfigurationSingleton) utl::Singleton<Configuration>;
%template(RandomGeneratorSingleton) utl::Singleton<RandomGenerator>;

%include <AugerUnits.h>
%include <BitTools.h>
%include <Configuration.h>
%include <KascadeEnergyConverter.h>
%include <LinearInterpolator.h>
%include <MathConstants.h>
%include <MathFunctions.h>
%include <PhysicalConstants.h>
%include <PhysicsFunctions.h>
%include <RandomGenerator.h>
%include <TimedClass.h>

%ignore utl::TimeStamp::operator = (const TimeStamp&);
%include <TimeStamp.h>

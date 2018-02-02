#include "Atmosphere.h"

#include <stdexcept>
#include <sstream>
#include <cmath>
#include <vector>
#include <iostream>

#include <AugerUnits.h>
#include <PhysicsFunctions.h>
#include <PhysicalConstants.h>


using namespace std;
using namespace utl;


Atmosphere::Atmosphere() {

  fMinHeight = 0.*m;
  fMaxHeight = 110000.*m;
  fTotDepth = Depth(fMinHeight,1.);

}


double Atmosphere::Depth (double h, double costh) const {

  const double  aatm[] = {-186.5562,-94.919,0.61289,0.0,0.01128292};
  const double  batm[] = {1222.6562,1144.9069,1305.5948,540.1778, 1.0};
  const double  catm[] = {994186.38,878153.55,636143.04,772170,1.e09};
  const double  hlay[] = {-1.e05,4.0e05,1.0e06,4.0e06,1.0e07};

  const double  height = h/cm;
  double  Xv = 0;

  if (height <= hlay[1])
    Xv = aatm[0] + batm[0] * exp((-height)/catm[0]);
  else if (height <= hlay[2])
    Xv = aatm[1] + batm[1] * exp((-height)/catm[1]);
  else if (height <= hlay[3])
    Xv = aatm[2] + batm[2] * exp((-height)/catm[2]);
  else if (height <= hlay[4])
    Xv = aatm[3] + batm[3] * exp((-height)/catm[3]);
  else if (height > hlay[4])
    Xv = aatm[4] - height / catm[4];

  return Xv/costh*g/cm2;

}


double Atmosphere::Height(double depth, double costh) const {

  const double aatm[] = {-186.5562,-94.919,0.61289,0.0,0.01128292};
  const double batm[] = {1222.6562,1144.9069,1305.5948,540.1778, 1.0};
  const double catm[] = {994186.38,878153.55,636143.04,772170,1.e09};
  const double thickl[] = {1036.1,631.1008,271.7001,3.039500,1.2829201e-03};

  const double Xv = depth*costh/g*cm2;
  double height = 0;

  if (Xv >= thickl[1])
    height = catm[0] * log(batm[0] / (Xv - aatm[0]) );
  else if (Xv >= thickl[2])
    height = catm[1] * log(batm[1] / (Xv - aatm[1]) );
  else if (Xv >= thickl[3])
    height = catm[2] * log(batm[2] / (Xv - aatm[2]) );
  else if (Xv >= thickl[4])
    height = catm[3] * log(batm[3] / (Xv - aatm[3]) );
  else if (Xv < thickl[4])
    height = (aatm[4] - Xv) *catm[4];

  return height*cm;

}


double Atmosphere::Pressure(double h) const {

  const double X = Depth(h,1);
  return X * kgEarth; // in Pascal

}


double Atmosphere::Temperature(double h) const {

  const int n = 7;
  const double  hlayer[n] = {0.*km, 11.*km, 20.*km, 32.*km, 47.*km,
                             51.*km, 71.*km};
  const double  Tatm[n] = {288.15, 216.65, 216.65, 228.65, 270.65,
                           270.65, 214.65};

  int i2 = 0;

  for (int i = 0; i < n; i ++) {
    const double height = hlayer[i];
    if (h > height) i2 = i + 1;
  }

  if (i2 == 0)
    i2 = 1;
  if (i2 > n)
    i2 = n-1;
  const int i1 = i2 - 1;

  const double T1 = Tatm[i1];
  const double T2 = Tatm[i2];
  const double h1 = hlayer[i1];
  const double h2 = hlayer[i2];

  const double T = T1 + (T2 - T1)/(h2 - h1) * (h - h1);

  return T*kelvin;

}

double Atmosphere::Density(double h) const {
  
  const double pressure = Pressure(h);
  const double temperature = Temperature(h);
  const double density = kDryAirMolarMass * pressure / (kMolarGasConstant * temperature);
  return density;
  
}

double Atmosphere::GetLifeTime(double h) const {
  
  double H = h;
  if(H > 12000*m) H = 12000*m; 

  const double fractionN2 = 0.78084;
  const double fractionO2 = 0.209476;
  const double fractionCD = 1.0 - fractionN2 - fractionO2;
  const double katt1 = 2e-30*cm3*cm3/s;
  const double katt2 = 8e-32*cm3*cm3/s;

  const double AirMolW = (kN2MolarMass*fractionN2 + kO2MolarMass*fractionO2 + kCO2MolarMass*fractionCD);
  
   const double N = (Density(H)*kAvogadro/AirMolW);
  
  const double tau = 1./(katt1*N*N*fractionO2*fractionO2 + katt2*N*N*fractionN2* fractionO2); 
  
  //if(tau > 1*microsecond)   
  //  return 7.*nanosecond;
  //else 
  //cout << "tau[ns] = " << tau /ns << endl;
  return tau;

}

//from R.C. Weast - Handbook of Chemistry and Physics 67th Edition (1986)
double RefrIndexLambda(double lambda) {

  const double l = lambda/angstrom;
  const double l2 = l*l;
  return 1. + (2726.43 + 12.288/(l2*1.e-8) + 0.3555/(l2*l2*1.e-16 )) *1.e-7;

}


//from Edlen Formula
double  RefrIndexEdlen(double lambda) {

  const double l = lambda/angstrom;// convert lambda from m to in Angstroms
  return 1. + (6432.8 + 2949810.0/(146- 2.*kPi/(l*l)) +
               25540.0/(41- 2.*kPi/(l*l))) * 1.e-8;

}


//F. Nerling Ph.D. Thesis pag. 23-24
double
Atmosphere::RefrIndex(double h, double phFreq)
  const {

  const double X = this->Pressure(h)/this->Pressure(0);

  if (phFreq < 0 )
    return 1. + 0.000283*X;
  else
    return 1. + ( RefrIndexLambda(kSpeedOfLight/phFreq) - 1.)*X;

}


double Atmosphere::GetTimeOfFlight(const double h1, const double h2,
                                   const double cosTheta,
                                   const double frequency)
  const  {

  map<double, LinearInterpolator<double>*>::const_iterator iter =
    fTimeOfFlightMap.find(frequency);
  if ( iter == fTimeOfFlightMap.end() ) {
    BuildCumulativeTOF(frequency);
    iter = fTimeOfFlightMap.find(frequency);
  }
  const LinearInterpolator<double>& cumulativeTOF = *(iter->second);
  
  return fabs(cumulativeTOF.Eval(h2) - cumulativeTOF.Eval(h1)) / cosTheta;

}


void
Atmosphere::BuildCumulativeTOF(const double frequency)
  const {

  vector<double> tofSums;

  size_t nSteps = (fMaxHeight - fMinHeight) / (50. * m);
  double dHeight = (fMaxHeight - fMinHeight) / nSteps;

  double tofSum = 0.;
  tofSums.push_back(tofSum);
  for (size_t i = 0; i < nSteps + 1; i++) {
    const double thisSpeed = kSpeedOfLight / RefrIndex(fMinHeight + i * dHeight,
                                                 frequency);
    const double thisDeltaT = dHeight / thisSpeed;

    tofSum += thisDeltaT;
    tofSums.push_back(tofSum);
  }

  LinearInterpolator<double>* tofInterpolator =
    new LinearInterpolator<double>(fMinHeight, dHeight, tofSums);

  fTimeOfFlightMap[frequency] = tofInterpolator;

}


// e.g. G.C. Medellin -- Antenna Noise Temperature Calculation
//double Atmosphere::MWSkyNoise(const double elevation, const double frequency)
double Atmosphere::MWSkyNoise(const double elevation) const { 

#warning Noise as a function of elevation still not implemented
  const double T0 = 4.*kelvin; 
  const double Tground = 300.*kelvin; 
  double Tbkg = T0;
  const double elevation_in_radians = elevation;// only for 90 deg elevation use 250 for MIDAS
  // simplest calculation of sky noise temperature (no dependence on frequency, no effect of humidity, clouds, galactic radiation, etc.)
  if (elevation_in_radians < kPi/2.) {Tbkg = T0/ cos(elevation_in_radians);}
  else {Tbkg = Tground;}
  if (Tbkg >= Tground) Tbkg = Tground;
  return Tbkg;

}


double Atmosphere::MoliereRadius(double depth, double cosTheta) const {

  const double millibar = 1e-3*bar;
  const double exponent = 1 / 5.25588;

  const double h = Height(depth, cosTheta);
  const double temperature = Temperature(h)/kelvin;
  const double pressure =  Pressure(h)/millibar;

  double correctedPressure =
    pressure - kgEarth * 2.* kRadiationLength * cosTheta/millibar;

  // for security. High in the atmosphere, low pressure
  // linear asymptotic behaviour to zero
  const double p_crit = 10;  // [millibar]
  if (correctedPressure < p_crit) {
    const double dp = pressure - correctedPressure;
    correctedPressure = pressure - dp * pressure / (p_crit + dp); // should never get zero
  }

  const double rm = 272.5 * temperature *
    pow (correctedPressure/pressure, exponent) / correctedPressure;

  return rm * meter;

}

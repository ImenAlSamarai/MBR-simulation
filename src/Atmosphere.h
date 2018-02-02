#ifndef __Atmosphere__
#define __Atmosphere__

#include <map>

#include <Singleton.h>
#include <LinearInterpolator.h>

#include <TVector3.h>

class Atmosphere : public utl::Singleton<Atmosphere> {

public:

  /// height given slant depth and cos(theta)
  double Height(double depth, double cosTheta) const;
  /// slant depth given height and cos(theta)
  double Depth(double height, double cosTheta) const;
  /// pressure given height
  double Pressure(double height) const;
  /// temperature given height
  double Temperature(double height) const;
  /// density given height
  double Density(double height) const;
  /// electrions lifetime
  double GetLifeTime(double height) const;
  /// maximum height of atmosphere
  double GetMaxHeight() const {return fMaxHeight;}
  /// minimum height of atmosphere
  double GetMinHeight() const {return fMinHeight;}
  /// maximum vertical depth
  double GetMaxVerticalDepth() const {return fTotDepth;}
  /// refractive index given height
  double RefrIndex(double height, double frequency) const;
  /// get time of flight from h1 to h2 taking into account refraction
  double GetTimeOfFlight(const double height1, const double height2,
                         const double cosTheta,
                         const double frequency) const;
  /// get time of flight from point p1 to p2 taking into account refraction
  double GetTimeOfFlight(const TVector3 p1, const TVector3 p2,
                         const double frequency) const {
    return GetTimeOfFlight(p1.Z(),p2.Z(),(p2-p1).CosTheta(),frequency);
  }
  /// microwave sky temperatire as a function of elevation and frequency
  //double MWSkyNoise(const double elevation, const double frequency) const;
  double MWSkyNoise(const double elevation) const;

  /// moliere radius as a function of depth and elevation
  double MoliereRadius(double depth, double cosTheta) const;

private:

  void BuildCumulativeTOF(const double frequency) const;

  friend class utl::Singleton<Atmosphere>;
  Atmosphere();

  double fMaxHeight;
  double fMinHeight;
  double fTotDepth;
  mutable std::map<double, utl::LinearInterpolator<double>*> fTimeOfFlightMap;

};

#endif //__Atmosphere__

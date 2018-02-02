#ifndef __ChannelGain__
#define __ChannelGain__

#include <limits>
#include <stdexcept>

#include <TH2D.h>
#include <TObject.h>
#include <TVector3.h>

#include <AugerUnits.h>
#include <MathConstants.h>


class ChannelGain : public TObject {
public:
 ChannelGain(TH2D* gainHist = NULL, double _fPhiOffset = 0.0) :
    fGainHist(gainHist),
    fThetaMin(0.0),
    fThetaMax(0.0),
    fPhiMin(0.0),
    fPhiMax(0.0),
    fPhiOffset(_fPhiOffset)
  {
    if (fGainHist != NULL) {
      // Set limits
      fThetaMin = fGainHist->GetXaxis()->GetXmin() * utl::radian;
      fThetaMax = fGainHist->GetXaxis()->GetXmax() * utl::radian;
      fPhiMin = fGainHist->GetYaxis()->GetXmin() * utl::radian;
      fPhiMax = fGainHist->GetYaxis()->GetXmax() * utl::radian;
    }
  }


  double Eval(const TVector3& vec) const
  {
    #if ROOT_VERSION_CODE < ROOT_VERSION(5,26,0)
    #error "ChannelGain needs ROOT >5.25 for TH2::Interpolate()."
    #endif // if ROOT_VERSION_CODE < ROOT_VERSION(5,26,0)

    if (fGainHist == NULL) {
      throw std::runtime_error("ChannelGain::Eval(): Tried to use not "
                               "initialised channel gain.");
    }

    if (!IsWithinLimits(vec)) {
      return std::numeric_limits<double>::min();
    }

    const double theta = vec.Theta() * utl::radian;
    const double phi = CorrectForPhiOffset(vec.Phi() * utl::radian);

    return fGainHist->Interpolate(theta, phi);
  }


  bool IsWithinLimits(const TVector3& vec) const {
    const double theta = vec.Theta() * utl::radian;
    const double phi = CorrectForPhiOffset(vec.Phi() * utl::radian);

    return ((theta >= fThetaMin) && (theta <= fThetaMax) &&
            (phi >= fPhiMin) && (phi <= fPhiMax));
  }


  double CorrectForPhiOffset(double phi) const {
    phi -= fPhiOffset;

    while (phi < -utl::kPi)
      phi += 2 * utl::kPi;

    while (phi > utl::kPi)
      phi -= 2 * utl::kPi;

    return phi;
  }


private:
  TH2D* fGainHist;

  // Limits of the fGainHist
  double fThetaMin, fThetaMax;
  double fPhiMin, fPhiMax;

  // An additional azimuth offset of the gain pattern
  double fPhiOffset;

  ClassDef(ChannelGain, 1);
};

#endif // ifndef __ChannelGain__

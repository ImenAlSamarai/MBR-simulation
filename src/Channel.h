#ifndef __Channel__
#define __Channel__

#include <string>
#include <vector>

#include <TObject.h>
#include <TRotation.h>
#include <TVector3.h>

#include "PhysicalConstants.h"

#include "ChannelGain.h"


class Antenna;


class Channel : public TObject {

public:

  Channel(size_t antennaId = 0);
  ~Channel();

  /// channel Id
  UInt_t GetId() const { return fId; }
  /// full angular beam width
  Double_t GetBeamWidth() const { return fBeamWidth; }
  /// position within antenna
  const TVector3& GetLocalPosition() const { return fPosition; }
  /// position within array
  const TVector3& GetGlobalPosition() const { return fPositionInGlobalCS; }
  /// pointing direction
  const TVector3& GetPointingDirection() const { return fPointingDirectionInGlobalCS; }
  /// band width
  Double_t GetBandWidth() const { return fBandWidth; }
  Double_t GetCenterFrequency() const { return fCenterFrequency; }
  Double_t GetCenterWavelength() const {
    return utl::kSpeedOfLight / fCenterFrequency;
  }

  Double_t GetNoise() const { return fNoise; }
  Double_t GetTraceBinWidth() const { return fTraceBinWidth;}
  Double_t GetTraceStartTime() const { return fTraceStartTime;}
  UInt_t GetNTraceBins() const { return fnTraceBins;}
  Double_t GetCableDelay() const { return fCableDelay;}

  // far field
  double GetDistanceToFarField() const { return fDistanceToFarField; }
  void SetDistanceToFarField(double r) { fDistanceToFarField = r; }

  // boresight gain
  double GetBoresightGain() const { return fBoresightGain; }
  double GetBoresightEffectiveArea() const;
  void SetBoresightGain(double);

  // gain pattern
  bool HasResponse() const { return fChannelGain; }
  void MakeResponse(std::string filename, const Antenna& antenna);
  double GetResponse(const TVector3&) const;
  double GetEffectiveArea(const TVector3&) const;

  /// elevation in antenna CS
  Double_t GetLocalElevation() const { return fElevation; }
  /// azimuth in antenna CS
  Double_t GetLocalAzimuth() const { return fAzimuth; }
  /// position of a point (global CS) within FOV
  bool IsInFarField(const TVector3&) const;
  bool IsInFOV(const TVector3&) const;
  double AngleToFOV(const TVector3&) const;


  void SetId(UInt_t val) { fId = val; }
  void SetBeamWidth(Double_t val) { fBeamWidth = val; }
  void SetBandWidth(Double_t val) { fBandWidth=val; }
  void SetCenterFrequency(Double_t val) { fCenterFrequency=val; }
  void SetNoise(Double_t val) { fNoise=val; }
  void SetTraceBinWidth(Double_t val)  { fTraceBinWidth=val;}
  void SetNTraceBins(UInt_t val)  { fnTraceBins=val;}
  void SetCableDelay(Double_t val)  { fCableDelay=val;}
  void SetTraceStartTime(Double_t val)  { fTraceStartTime=val;}
  /// local antenna coordinates
  void SetLocalPosition(const TVector3& vec)  { fPosition=vec; }
  /// local antenna coordinates
  void SetLocalElevation(Double_t val)  { fElevation=val; }
  /// local antenna coordinates
  void SetLocalAzimuth(Double_t val)  { fAzimuth=val; }

  /// global coordinates
  void SetPointingDirection(const TVector3& val)
  { fPointingDirectionInGlobalCS = val; }
  /// global coordinates
  void SetGlobalPosition(const TVector3& val)
  { fPositionInGlobalCS = val; }

  /// CS transformations
  void RotateFromChannelToAntenna(TVector3&) const;
  void RotateFromAntennaToChannel(TVector3&) const;


private:

  void InitTrafos() const;

  UInt_t fId;
  UInt_t fAntennaId;

  Double_t fBeamWidth;
  Double_t fBoresightGain;
  Double_t fBandWidth;
  Double_t fCenterFrequency;
  Double_t fNoise;
  Double_t fElevation;
  Double_t fAzimuth;
  Double_t fCableDelay;
  Double_t fTraceBinWidth;
  Double_t fTraceStartTime;
  Double_t fDistanceToFarField;
  UInt_t   fnTraceBins;
  TVector3 fPosition;
  TVector3 fPointingDirectionInGlobalCS;
  TVector3 fPositionInGlobalCS;
  ChannelGain* fChannelGain;

  mutable TRotation fLocalToGlobal; //!
  mutable TRotation fGlobalToLocal; //!
  mutable bool fRotationInitialized; //!

  ClassDef(Channel, 2);

};

#endif  // ifndef __Channel__

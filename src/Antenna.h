#ifndef __Antenna_H
#define __Antenna_H

#include <TObject.h>
#include <TVector3.h>
#include <TRotation.h>
#include "Channel.h"
#include <vector>


class Antenna  : public TObject {

public:

  Antenna();

  UInt_t GetId() const { return fId; }
  Double_t GetDiameter() const { return fDiameter; }
  Double_t GetElevation() const { return fElevation; }
  Double_t GetAzimuth() const { return fAzimuth; }
  const TVector3& GetPosition() const { return fPosition; }
  TVector3& GetPosition() { return fPosition; }
  void AddChannel(const Channel& chan) { fChannels.push_back(chan); }
  UInt_t GetNumberOfChannels() const { return fChannels.size(); }
  const std::vector<Channel>& GetChannels() const { return fChannels;}
  const Channel& GetChannel(unsigned int channelId) const;
  Double_t GetResponse(unsigned int channelId, const TVector3& vec) const;


  void SetId(UInt_t val) { fId = val; }
  void SetDiameter(Double_t val)  {fDiameter=val; }
  void SetElevation(Double_t val)  {fElevation=val; }
  void SetAzimuth(Double_t val)  {fAzimuth=val; }
  void SetCameraAzimuth(Double_t val)  { fCameraAzimuth = val; }
  void SetPosition(const TVector3& vec)  {fPosition=vec; }

  /// get maximum opening angle
  double GetMaxTheta() const;

  // far field of channels
  void SetDistanceToFarField(double r);

  void RotateFromArrayToAntenna(TVector3& vec) const
  {TransformFromArrayToAntenna(vec, true);}
  void RotateFromAntennaToArray(TVector3& vec) const
  {TransformFromAntennaToArray(vec, true);}

  void TransformFromArrayToAntenna(TVector3&, bool onlyRotation=false) const;
  void TransformFromAntennaToArray(TVector3&, bool onlyRotation=false) const;

private:

  void InitTrafos() const;

  UInt_t fId;
  Double_t fDiameter;
  Double_t fElevation;
  Double_t fAzimuth;
  Double_t fCameraAzimuth;

  TVector3 fPosition;
  std::vector<Channel> fChannels;

  // max angular dimension
  mutable Double_t fMaxTheta; //!

  mutable TRotation fLocalToGlobal; //!
  mutable TRotation fGlobalToLocal; //!
  mutable bool fRotationInitialized; //!

  ClassDef(Antenna, 1);

};

#endif

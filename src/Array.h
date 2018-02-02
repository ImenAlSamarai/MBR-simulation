#ifndef __Array_H
#define __Array_H

#include <TObject.h>
#include <TVector3.h>
#include <map>

#include "Station.h"


class Array  : public TObject {

public:
  typedef std::map<int, Station>::const_iterator ConstStationIterator;

public:

  Array();

  Double_t GetHeight() const { return fHeight; }
  Double_t GetPhi() const { return fPhi; }
  Double_t GetSignalTimeDelay() const { return fSignalTimeDelay; }
  ConstStationIterator StationsBegin() const
  { return fStations.begin(); }
  ConstStationIterator StationsEnd() const
  { return fStations.end(); }
  unsigned int GetNumberOfStations() const
  { return fStations.size(); }
  const TVector3& GetStationPosition(int id) const
  { return fStations.find(id)->second.GetPosition(); }
  Station& GetStation(int id)
  { return fStations.find(id)->second; }
  const Station& GetStation(int id) const
  { return fStations.find(id)->second; }

  void SetHeight(Double_t val)  { fHeight=val; }
  void SetPhi(Double_t val)  { fPhi=val; }
  void SetSignalTimeDelay(Double_t val)  { fSignalTimeDelay=val; }
  void AddStation(const Station& station) {
    fStations[station.GetId()] = station;
  }

  void RotateFromArrayToGeographical(TVector3& v) const {
    v.SetPhi(v.Phi() + fPhi);
  }

  void RotateFromGeographicalToArray(TVector3& v) const {
    v.SetPhi(v.Phi() - fPhi);
  }

private:

  Double_t fHeight;
  Double_t fPhi;
  Double_t fSignalTimeDelay;
  Int_t fTriggerStationId;

  std::map<int, TVector3> fStationPositions;  // Deprecated since V2

  std::map<int, Station> fStations;

  ClassDef(Array, 2);

};

#endif

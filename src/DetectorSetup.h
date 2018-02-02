#ifndef __DetectorSetup_H
#define __DetectorSetup_H

#include <TObject.h>
#include <vector>
#include "Antenna.h"
#include "Array.h"

class DetectorSetup  : public TObject { 

public:

  DetectorSetup();

  std::vector<Antenna>& GetAntennas() { return fAntennas;}
  const std::vector<Antenna>& GetAntennas() const { return fAntennas;}
  const Antenna& GetAntenna(unsigned int antennaId) const;

  Array& GetArray() { return fArray; }
  const Array& GetArray() const { return fArray; }
  void AddAntenna(const Antenna& antenna) 
  { fAntennas.push_back(antenna); }

  /// get min and max X coordinate
  double GetMinX() const;
  double GetMaxX() const;
  /// get min and max Y coordinate
  double GetMinY() const;
  double GetMaxY() const;

private:

  void FindBounds() const;
  std::vector<Antenna> fAntennas;
  Array fArray;

  // max. array dimensions (the funny comment 
  // is a cryptic sign for ROOT not to stream this stuff)
  mutable Double_t fMinX; //!
  mutable Double_t fMaxX; //!
  mutable Double_t fMinY; //!
  mutable Double_t fMaxY; //!

  ClassDef(DetectorSetup, 1);

};

#endif

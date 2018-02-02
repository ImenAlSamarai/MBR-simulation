#include "DetectorSetup.h"
#include <stdexcept>
#include <sstream>
using namespace std;

ClassImp(DetectorSetup)


DetectorSetup::DetectorSetup() :
  fMinX(1.e99),
  fMaxX(-1.e99),
  fMinY(1.e99),
  fMaxY(-1.e99)
{

}


/// get min and max X coordinate
double DetectorSetup::GetMinX() const {
  if (fMinX > 1.e90)
    FindBounds();
  return fMinX;
}


double DetectorSetup::GetMaxX() const {
  if (fMaxX < -1.e90)
    FindBounds();
  return fMaxX;
}


/// get min and max Y coordinate
double DetectorSetup::GetMinY() const {
  if (fMinY > 1.e90)
    FindBounds();
  return fMinY;
}


double DetectorSetup::GetMaxY() const {
  if (fMaxY < -1.e90)
    FindBounds();
  return fMaxY;
}

void DetectorSetup::FindBounds() const {

  bool first = true;
  for ( Array::ConstStationIterator iStation = fArray.StationsBegin();
       iStation != fArray.StationsEnd();
       ++iStation) {

    const TVector3& stationPosition = iStation->second.GetPosition();
    const double x = stationPosition.X();
    const double y = stationPosition.Y();

    if (first) {
      first = false;
      fMinX = x;
      fMaxX = x;
      fMinY = y;
      fMaxY = y;
    } 
    else {

      if (x > fMaxX) 
        fMaxX = x;
      if (x < fMinX) 
        fMinX = x;
      if (y > fMaxY) 
        fMaxY = y;
      if (y < fMinY) 
        fMinY = y;
    }

  }

  for ( vector<Antenna>::const_iterator iter = fAntennas.begin();
        iter != fAntennas.end();
        ++iter ) {

    const double x = iter->GetPosition().X();
    const double y = iter->GetPosition().Y();

    if (first) {
      first = false;
      fMinX = x;
      fMaxX = x;
      fMinY = y;
      fMaxY = y;
    } 
    else {
      if (x > fMaxX) 
        fMaxX = x;
      if (x < fMinX) 
        fMinX = x;
      if (y > fMaxY) 
        fMaxY = y;
      if (y < fMinY) 
        fMinY = y;
    }
  }

}

const Antenna& DetectorSetup::GetAntenna(unsigned int antennaId) const {
  for ( vector<Antenna>::const_iterator iter = fAntennas.begin();
        iter != fAntennas.end(); ++iter )  
    if ( iter->GetId() == antennaId ) 
      return *iter;
  
  ostringstream errMsg;
  errMsg << " DetectorSetup:GetAntenna() - Error: request for "
         << "nonexisting antenna " 
         << antennaId;
  throw std::runtime_error(errMsg.str());
  
}

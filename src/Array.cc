#include "Array.h"

ClassImp(Array)

Array::Array() :
  fHeight(0),
  fPhi(0)
{

}


void Array::Streamer(TBuffer& R__b)
{
  if (R__b.IsReading()) {
    unsigned int R__s, R__c;
    Version_t R__v = R__b.ReadVersion(&R__s, &R__c);

    R__b.ReadClassBuffer(Array::Class(), this, R__v, R__s, R__c);

    if (R__v < 2) {
      // Manually transfer data from (deprecated) fStationPositions to
      // fStations
      for (std::map<int, TVector3>::const_iterator iter =
             fStationPositions.begin();
           iter != fStationPositions.end(); ++iter) {
        Station station;

        station.SetId(iter->first);
        station.SetPosition(iter->second);

        AddStation(station);
      }

      // Clear the deprecated member just to make sure that nobody relies on it
      fStationPositions.clear();
    }
  } else {
    R__b.WriteClassBuffer(Array::Class(), this);
  }
}

#include "Station.h"

ClassImp(Station)


Station::Station()
: fId(0),
  fPosition(TVector3(0.0, 0.0, 0.0)),
  fTriggerTimeRange(std::pair<Double_t, Double_t>(0.0, 0.0)),
  fTriggerTimeOffset(0.0)
{
}

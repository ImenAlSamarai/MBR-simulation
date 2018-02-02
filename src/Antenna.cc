#include "Antenna.h"

#include <iostream>
#include <stdexcept>
#include <sstream>

#include <AugerUnits.h>
#include <MathConstants.h>
#include <PhysicalConstants.h>

using namespace utl;
using namespace std;

ClassImp(Antenna)


Antenna::Antenna() :
  fId(0),
  fDiameter(0),
  fElevation(0),
  fAzimuth(0),
  fCameraAzimuth(0),
  fPosition(TVector3(0.,0.,0.)),
  fMaxTheta(-1e99),
  fRotationInitialized(false)
{

}

double Antenna::GetMaxTheta() const {
  if (fMaxTheta < 0. ) {

    for ( std::vector<Channel>::const_iterator iter = fChannels.begin();
          iter != fChannels.end(); ++iter ) {
      const double thetaChan = kPi/2.-iter->GetLocalElevation();
      const double halfWidth = iter->GetBeamWidth()/2.;
      if ( thetaChan+halfWidth > fMaxTheta )
        fMaxTheta = thetaChan+halfWidth;
    }
  }

  return fMaxTheta;
}


const Channel& Antenna::GetChannel(unsigned int channelId) const {
  for (unsigned int i = 0; i < fChannels.size(); i++) {
    if ( fChannels[i].GetId() == channelId ) {
      return fChannels[i];
    }
  }

  ostringstream errMsg;
  errMsg << " Antenna::GetChannel() - Error: request for nonexisting channel "
         << channelId;
  throw std::runtime_error(errMsg.str());

}


Double_t Antenna::GetResponse(unsigned int channelId, const TVector3& vec)
  const {

  return GetChannel(channelId).GetResponse(vec);

}


/*
  Sets the distance at which the far field starts for  all channels.
 */
void Antenna::SetDistanceToFarField(double r)
{

  for (unsigned int i = 0; i < fChannels.size(); i++) {
    fChannels[i].SetDistanceToFarField(r);
  }

}


void Antenna::TransformFromAntennaToArray(TVector3& vec, bool onlyRotation)
  const {
  if ( !fRotationInitialized )
    InitTrafos();

  vec.Transform(fLocalToGlobal);
  if ( ! onlyRotation )
    vec+=fPosition;
}


void Antenna::TransformFromArrayToAntenna(TVector3& vec, bool onlyRotation)
  const {
  if ( !fRotationInitialized )
    InitTrafos();

  if ( ! onlyRotation )
    vec-=fPosition;
  vec.Transform(fGlobalToLocal);
}


void Antenna::InitTrafos() const {
  fRotationInitialized = true;
  fLocalToGlobal = TRotation();

  TVector3 axis(0., 0., 1.);
  axis.SetTheta(kPi / 2. - fElevation);
  axis.SetPhi(fAzimuth);

  TVector3 rotationAxis = TVector3(0., 0., 1.).Cross(axis);

  fLocalToGlobal.Rotate(fCameraAzimuth, TVector3(0., 0., 1.));
  fLocalToGlobal.Rotate(kPi / 2. - fElevation, rotationAxis);

  fGlobalToLocal = fLocalToGlobal.Inverse();
}


//double Antenna::GetTAnt(unsigned int channelId) {
//  Channel& channel=GetChannel(channelId);
  // g(theta,phi) = channel.  


//}

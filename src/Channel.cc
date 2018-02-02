#include "Channel.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

#include <TH2D.h>

#include <tinyxml.h>
#include <tinyxmlCromeUtilities.h>

#include <MathConstants.h>
#include <MathFunctions.h>
#include <AugerUnits.h>
#include <Detector.h>
#include <DetectorSetup.h>
#include "Antenna.h"
#include "ChannelGain.h"

using namespace utl;
using namespace std;

ClassImp(Channel)


Channel::Channel(size_t antennaId) :
  fId(0),
  fAntennaId(antennaId),
  fBeamWidth(0),
  fBoresightGain(0.),
  fBandWidth(0),
  fCenterFrequency(0),
  fNoise(0),
  fElevation(0),
  fAzimuth(0),
  fCableDelay(0),
  fTraceBinWidth(0.),
  fDistanceToFarField(0.),
  fPosition(TVector3(0.,0.,0.)),
  fPointingDirectionInGlobalCS(TVector3(0.,0.,0.)),
  fPositionInGlobalCS(TVector3(0.,0.,0.)),
  fChannelGain(NULL),
  fRotationInitialized(false)
{

}

Channel::~Channel() {
}


void
Channel::RotateFromChannelToAntenna(TVector3& vec) const {
  if ( !fRotationInitialized )
    InitTrafos();

  vec.Transform(fLocalToGlobal);
}


void
Channel::RotateFromAntennaToChannel(TVector3& vec) const {
  if ( !fRotationInitialized )
    InitTrafos();

  vec.Transform(fGlobalToLocal);
}

void Channel::InitTrafos() const {
  fRotationInitialized = true;
  fLocalToGlobal = TRotation();
  fLocalToGlobal.RotateY(kPi/2.-fElevation);
  fLocalToGlobal.RotateZ(fAzimuth);
  fGlobalToLocal = fLocalToGlobal.Inverse();
}


/*
  Tests whether point, given in the array CS, is inside the field of view of
  the channel.

  When the channel has a gain pattern, its boundaries are the limits of the FoV.
  Otherwise, the FoV is a cone of (full) opening angle fBeamWidth around the
  boresight axis.
 */
bool
Channel::IsInFOV(const TVector3& point)
  const {

  if ( HasResponse() ) {
    // ChannelGain::IsWithinLimits expects a point in the antenna CS
    const DetectorSetup& detectorSetup = Detector::GetInstance().GetSetup();
    const Antenna& antenna = detectorSetup.GetAntenna(fAntennaId);

    // Transform point to antenna coordinate system
    TVector3 pointInAntennaCS(point);
    antenna.TransformFromArrayToAntenna(pointInAntennaCS);

    return fChannelGain->IsWithinLimits(pointInAntennaCS);
  } else {
    // AngleToFOV expects a point in the global CS
    return AngleToFOV(point) < fBeamWidth / 2.;
  }

}


bool Channel::IsInFarField(const TVector3& point /* array cs */)
  const {

  TVector3 channelToPointVector = point - fPositionInGlobalCS;
  return channelToPointVector.Mag() > fDistanceToFarField;

}


double
Channel::AngleToFOV(const TVector3& point /* array cs */)
  const {

  TVector3 channelToPointVector = point - fPositionInGlobalCS;
  return channelToPointVector.Angle(fPointingDirectionInGlobalCS);

}


/*
  Returns the gain factor in the direction of point given in the array CS.
 */
double Channel::GetResponse(const TVector3& point /* array cs */)
const {

  if( !HasResponse() ) {
    throw runtime_error("Channel::GetResponse(): Requested unknown gain.");
  }
  
  const DetectorSetup& detectorSetup = Detector::GetInstance().GetSetup();
  const Antenna& antenna = detectorSetup.GetAntenna(fAntennaId);
  
  // Transform point to antenna coordinate system
  TVector3 pointInAntennaCS(point);
  antenna.TransformFromArrayToAntenna(pointInAntennaCS);
  
  
  return fChannelGain->Eval(pointInAntennaCS);

}


double Channel::GetBoresightEffectiveArea()
const {

  const double gain = fBoresightGain;
  const double wavelength = GetCenterWavelength();
  return gain * wavelength * wavelength / (4.0 * kPi);

}


void Channel::SetBoresightGain(double gain)
{

  if ( HasResponse() ) {
    throw runtime_error("Channel::SetBoresightGain(): "
                        "Boresight gain is fixed for channels with gain "
                        "table.");
  }

  fBoresightGain = gain;

}


double Channel::GetEffectiveArea(const TVector3& point /* array cs */)
const {

  // Check whether the point is in the field of view
  if ( !IsInFOV(point) ) {
    // The point is outside the field of view, the channel is not sensitive
    return 0.0; //uncomment for real setup this is just for Gorham
  cout << "is not in FOV"<<endl;
  }

  // Check whether the point is in the far field
  if ( !IsInFarField(point) ) {
    // We don't have gain tables for the near field yet. Ignoring any emission
    // in the near field will produce a lower limit for the total flux.
    return 0.0; //uncomment for real setup this is just for Gorham
  }

  // Get the gain in the direction of the point
  double gain;
  if ( HasResponse() ) {
    // We have a radiation pattern for this channel: query it for this point
    gain = GetResponse(point);
    //cout << "Response is " << gain << endl;
  } else {
    // We don't have a radiation pattern for this channel: use the constant
    // boresight gain
    gain = fBoresightGain;
    //cout << " Using Boresight for response => " << gain << endl;
  }

  const double wavelength = GetCenterWavelength();
  return gain * wavelength * wavelength / (4.0 * kPi);
  
}


void Channel::MakeResponse(string filename, const Antenna& antenna)
{

  if ( HasResponse() ) {
    throw runtime_error("Channel::MakeResponse(): Tried to overwrite existing"
                        "response.");
  }

  TiXmlDocument doc(filename);
  if ( !doc.LoadFile() ) {
    throw runtime_error("Channel::MakeResponse(): Error while loading " +
                        filename + ".");
  }

  const TiXmlElement* topBranch = doc.FirstChildElement("ChannelGain");
  if (!topBranch) {
    throw runtime_error("Channel::MakeReponse(): Cannot find ChannelGain "
                        "branch.");
  }

  // Get phi limits and number of steps
  const TiXmlElement* phiBranch = GetFirstChild(topBranch, "phi");
  const double phiMin =
    GetDoubleValue(GetFirstChild(phiBranch, "min")) * degree;
  const double phiMax =
    GetDoubleValue(GetFirstChild(phiBranch, "max")) * degree;
  const unsigned int phiSteps =
    GetUnsignedValue(GetFirstChild(phiBranch, "steps"));

  // Generate the edges of equidistant bins covering the range
  // [phiMin - dPhi / 2., phiMax + dPhi / 2.], so that each bin will be
  // centered at the correct value.
  const double dPhi = (phiMax - phiMin) / (phiSteps - 1);
  const vector<double> phiBins = GenerateBinEdges((phiMin - dPhi / 2.) / radian,
                                                  (phiMax + dPhi / 2.) / radian,
                                                  phiSteps);

  // Get theta limits and number of steps
  const TiXmlElement* thetaBranch = GetFirstChild(topBranch, "theta");
  const double thetaMin =
    GetDoubleValue(GetFirstChild(thetaBranch, "min")) * degree;
  const double thetaMax =
    GetDoubleValue(GetFirstChild(thetaBranch, "max")) * degree;
  const unsigned int thetaSteps =
    GetUnsignedValue(GetFirstChild(thetaBranch, "steps"));

  // Generate the edges of equidistant bins covering the range
  // [thetaMin - dTheta / 2., thetaMax + dTheta / 2.], so that each bin will be
  // centered at the correct value.
  const double dTheta = (thetaMax - thetaMin) / (thetaSteps - 1);
  const vector<double> thetaBins =
    GenerateBinEdges((thetaMin - dTheta / 2.) / radian,
                     (thetaMax + dTheta / 2.) / radian,
                     thetaSteps);

  ostringstream gainHistTitle;
  gainHistTitle << "hGain_" << antenna.GetId() << "_" << fId;
  TH2D* hGain = new TH2D(gainHistTitle.str().c_str(), "",
                         thetaSteps, &thetaBins[0],
                         phiSteps, &phiBins[0]);

  unsigned int row = 1;
  const TiXmlElement* gainBranch = GetFirstChild(topBranch, "gain");
  const TiXmlElement* rowBranch = GetFirstChild(gainBranch, "row");
  while ( rowBranch ) {
    // Read gain in dBi and fill the histogram with factors
    const vector<double> gain = GetDoubleTable(rowBranch);

    for (unsigned int col = 1; col < phiSteps + 1; col++) {
      hGain->SetBinContent(row, col, pow(10., gain[col - 1] / 10.));
     // cout << " ************ " << gain[col - 1] << " -> " << dBmToPower(gain[col - 1]) << "  ------- " << pow(10., gain[col - 1] / 10.) << endl; 

    }
    rowBranch = rowBranch->NextSiblingElement("row");
    row++;
  }

  // Set boresight gain to maximum gain in the table
  fBoresightGain = hGain->GetMaximum();

  fChannelGain = new ChannelGain(hGain, fAzimuth);

}

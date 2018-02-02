#ifndef __Station__
#define __Station__

#include <utility>

#include <TObject.h>
#include <TVector3.h>


class Station : public TObject {
 public:
  Station();

  UInt_t GetId() const { return fId; }
  void SetId(UInt_t id) { fId = id; }

  // Position of the station in array coordinates
  const TVector3& GetPosition() const { return fPosition; }
  void SetPosition(const TVector3& position) { fPosition = position; }

  // Range of the expected trigger time relative to the CROME trigger
  std::pair<Double_t, Double_t> GetTriggerTimeRange() const {
    return fTriggerTimeRange;
  }
  void SetTriggerTimeRange(const std::pair<Double_t, Double_t>& triggerTimeRange) {
    fTriggerTimeRange = triggerTimeRange;
  }

  // Offset between trigger condition and actual rise of trigger signal
  Double_t GetTriggerTimeOffset() const { return fTriggerTimeOffset; }
  void SetTriggerTimeOffset(Double_t offset) { fTriggerTimeOffset = offset; }

 private:

  UInt_t fId;

  TVector3 fPosition;

  std::pair<Double_t, Double_t> fTriggerTimeRange;

  Double_t fTriggerTimeOffset;

  ClassDef(Station, 2);
};

#endif

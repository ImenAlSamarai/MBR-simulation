#ifndef __Detector__
#define __Detector__

#include <Singleton.h>
#include <string>
#include <stdexcept>

class DetectorSetup;
class Antenna;
class Channel;
class ChannelGain;
class EventFile;
class TiXmlElement;

class Detector : public utl::Singleton<Detector> {

public:

  void Init();

  void DeInit();

  void InitFromFile(EventFile&);

  const DetectorSetup& GetSetup() const {
    if ( fDetectorSetup == NULL ) {
      throw std::runtime_error("Detector::GetSetup(): "
                               "Trying to get uninitialized detector setup.");
    }

    return *fDetectorSetup;
  }

private:

  friend class utl::Singleton<Detector>;
  Detector();
  ~Detector();

private:

  bool InitArray(const char* arrayType);
  bool AddAntenna(const TiXmlElement*);
  void AddChannel(const TiXmlElement*, Antenna&);
  bool SetDishProperties(const char* dishType, Antenna&);
  void SetChannelProperties(const std::string channelType, Channel&);
  DetectorSetup* fDetectorSetup;
  int fVerbosity;

};

#endif

# MBR-simulation: Molecular Bremsstrahlung Radiation Detection Simulation

## Overview

This C++ software package simulates the detection capabilities of radio antenna arrays designed to detect **Molecular Bremsstrahlung Radiation (MBR)** from cosmic ray-induced extensive air showers (EAS). The simulation implements the phenomenological model described in the seminal paper:

> **I. Al Samarai et al.** "An Estimate of the Spectral Intensity Expected from the Molecular Bremsstrahlung Radiation in Extensive Air Showers" *Astropart.Phys.* **67** (2015) 26-32

## Scientific Background

### Molecular Bremsstrahlung Radiation in Air Showers

When ultra-high energy cosmic rays (UHECR, E > 10¹⁷ eV) interact with Earth's atmosphere, they create extensive air showers containing billions of secondary particles. As these particles traverse the atmosphere, they ionize air molecules, leaving behind low-energy electrons (~1-10 eV) that subsequently undergo quasi-elastic collisions with atmospheric molecules (N₂, O₂).

The **Molecular Bremsstrahlung Radiation** emerges from these collisions through:

1. **Ionization Process**: High-energy shower particles ionize atmospheric molecules
2. **Electron Thermalization**: Ionization electrons lose energy through successive collisions
3. **Bremsstrahlung Emission**: Accelerated electrons in molecular electric fields emit photons

### Theoretical Framework

#### Spectral Intensity Model

The differential spectral intensity of MBR photons at ground level is given by:

```
I(ν) = (e²/4πε₀c³) × ∫∫∫ [n_e(r,t) × σ_br(E_e,ν) × ρ_mol(r)] dr dt dE_e
```

Where:
- `I(ν)`: Spectral intensity at frequency ν [W m⁻² Hz⁻¹]
- `n_e(r,t)`: Electron density distribution in space and time [m⁻³]
- `σ_br(E_e,ν)`: Bremsstrahlung cross-section for electron energy E_e [m²]
- `ρ_mol(r)`: Molecular density at position r [m⁻³]

#### Key Emission Properties

- **Frequency Range**: 1-20 GHz (optimal for atmospheric transparency)
- **Angular Distribution**: Nearly isotropic (unlike geomagnetic or Askaryan emission)
- **Intensity**: ~5×10⁻²⁶ W m⁻² Hz⁻¹ at 10 km from shower core (10¹⁷·⁵ eV proton)
- **Polarization**: Unpolarized due to random collision orientations

### Competing Emission Mechanisms

1. **Geomagnetic Emission**: Coherent curvature radiation from deflected charged particles
2. **Askaryan Effect**: Coherent Cherenkov radiation from charge excess in shower front
3. **Fluorescence**: UV photons from de-excited nitrogen molecules

**MBR Advantages**:
- 24/7 detection capability (no moonless night requirement)
- Atmospheric transparency at GHz frequencies
- Direct proportionality to energy deposition

## Software Architecture

### Core Components

#### 1. Detector Simulation (`Detector.cc/h`)
- **Purpose**: Models configurable radio antenna arrays
- **Key Features**:
  - XML-based antenna configuration
  - Multi-channel per antenna support
  - Gain pattern integration
  - Field-of-view calculations

```cpp
class Detector {
    bool AddAntenna(const TiXmlElement* antennaBranch);
    void SetChannelProperties(string channelType, Channel& channel);
    bool InitArray(const char* arrayType);
};
```

#### 2. Channel Response (`Channel.cc/h`)
- **Purpose**: Simulates individual antenna channel characteristics
- **Key Features**:
  - Frequency-dependent gain patterns
  - Effective area calculations
  - Near/far-field transitions
  - Noise temperature modeling

```cpp
class Channel {
    double GetEffectiveArea(const TVector3& point);
    bool IsInFOV(const TVector3& point);
    double GetResponse(const TVector3& point);
};
```

#### 3. Atmospheric Model (`Atmosphere.cc/h`)
- **Purpose**: Implements realistic atmospheric conditions
- **Key Features**:
  - US Standard Atmosphere (1976)
  - Pressure/temperature/density profiles
  - Refractive index calculations
  - Microwave sky noise modeling

```cpp
class Atmosphere {
    double Density(double height);           // ρ(h) [kg/m³]
    double RefrIndex(double h, double freq); // n(h,ν)
    double MWSkyNoise(double elevation);     // T_sky [K]
};
```

### Mathematical Implementation

#### Atmospheric Density Profile

The simulation uses a 5-layer exponential atmosphere model:

```
ρ(h) = ρ₀ × exp(-h/H_i)  for layer i
```

Where `H_i` are the scale heights for different atmospheric regions.

#### Antenna Effective Area

The effective area calculation follows:

```
A_eff(θ,φ) = G(θ,φ) × λ²/(4π)
```

Where:
- `G(θ,φ)`: Antenna gain pattern
- `λ`: Wavelength at center frequency
- `θ,φ`: Spherical coordinates

#### Signal-to-Noise Ratio

The detection threshold is determined by:

```
SNR = (P_signal × t_integration) / (k_B × T_sys × B)
```

Where:
- `P_signal`: Received power from MBR
- `T_sys`: System noise temperature
- `B`: Receiver bandwidth
- `t_integration`: Integration time

## Key Features & Novelties

### 1. **Phenomenological MBR Model**
- First comprehensive simulation of molecular bremsstrahlung from air showers
- Incorporates realistic electron energy distributions
- Accounts for atmospheric absorption and scattering

### 2. **Configurable Array Geometries**
- Support for arbitrary antenna layouts
- Real-world antenna patterns from measurement data
- Multi-frequency channel simulation

### 3. **Atmospheric Effects**
- Detailed atmospheric model including:
  - Pressure/temperature gradients
  - Molecular composition variations
  - Refractive index frequency dependence

### 4. **Detection Optimization**
- Field-of-view calculations for optimal pointing
- Noise budget analysis
- Signal processing chain simulation

## Installation & Usage

### Prerequisites
```bash
# Required dependencies
- ROOT (CERN data analysis framework)
- CMake >= 3.0
- C++11 compatible compiler
- TinyXML library
- Python (for interface, optional)
```

### Building the Software
```bash
git clone https://github.com/ImenAlSamarai/MBR-simulation.git
cd MBR-simulation
mkdir build && cd build
cmake ..
make -j4
```

### Python Interface
The software includes SWIG-generated Python bindings for high-level analysis:

```python
import MBRSimulation as mbr

# Initialize detector
detector = mbr.Detector()
detector.Init()

# Set atmospheric conditions
atmosphere = mbr.Atmosphere()

# Simulate antenna response
channel = detector.GetChannel(antenna_id, channel_id)
effective_area = channel.GetEffectiveArea(source_position)
```

## Configuration Files

### Antenna Configuration (`antenna.xml`)
```xml
<MicrowaveDetector>
    <array type="GIGAS"/>
    <antenna id="1">
        <elevation>45.0</elevation>
        <azimuth>0.0</azimuth>
        <dish type="ParabolicReflector"/>
        <channel id="1" type="LNA_1GHz">
            <centerFrequency>1.4e9</centerFrequency>
            <bandWidth>100e6</bandWidth>
            <gain>15.0</gain>
        </channel>
    </antenna>
</MicrowaveDetector>
```

## Scientific Applications

### 1. **EASIER/GIGAS Experiments**
This simulation directly supported the design of:
- **GIGAS**: Gigahertz Identification of Giant Air Showers
- **EASIER**: Extensive Air Shower Identification using Electron Radiometers
- Deployed at Pierre Auger Observatory (Argentina)

### 2. **Next-Generation Detectors**
- Optimization of large-scale radio arrays
- Cost-benefit analysis for UHECR detection
- Systematic uncertainty evaluation

### 3. **Cross-Calibration Studies**
- Comparison with fluorescence detection
- Validation of energy reconstruction methods
- Independent composition measurements

## Performance Benchmarks

### Expected Signal Levels
For a 10¹⁸ eV proton shower at 10 km distance:

| Parameter | Value | Unit |
|-----------|-------|------|
| Peak spectral intensity | ~10⁻²⁵ | W m⁻² Hz⁻¹ |
| Optimal frequency | 1-3 | GHz |
| Signal duration | ~10 | ns |
| Angular spread | ~±30° | degrees |

### Detection Thresholds
With modern low-noise amplifiers:

| Array Size | Sensitivity | Energy Threshold |
|------------|-------------|------------------|
| 7 antennas | 3σ | 3×10¹⁸ eV |
| 61 antennas | 5σ | 10¹⁸ eV |
| 150 antennas | 10σ | 5×10¹⁷ eV |

## Validation & Experimental Results

### Laboratory Tests
- **SLAC T471**: First detection of microwave emission from EM showers
- **AMY Experiment**: Detailed spectrum measurements at BTF (Frascati)
- **Beam Test Facilities**: Controlled environment validation

### Field Deployments
- **AMBER**: Prototype imaging dish antenna
- **MIDAS**: Microwave Detection of Air Showers
- **GIGAS Arrays**: Operational at Auger Observatory

## Limitations & Future Developments

### Current Limitations
1. **Coherence Effects**: Potential suppression below collision frequency (~THz)
2. **Background Rejection**: RFI mitigation in populated areas  
3. **Atmospheric Variations**: Weather-dependent sensitivity
4. **Systematic Uncertainties**: Cross-section uncertainties (~30%)

### Future Enhancements
1. **Machine Learning**: AI-based signal recognition
2. **Multi-messenger**: Integration with gravitational wave detectors
3. **Space-based**: Satellite constellation for global coverage
4. **Quantum Effects**: Coherent emission mechanisms

## References

### Primary Publications
1. **Al Samarai, I. et al.** (2015) "An Estimate of the Spectral Intensity Expected from the Molecular Bremsstrahlung Radiation in Extensive Air Showers" *Astropart.Phys.* **67**, 26-32
2. **Gaïor, R. et al.** (2018) "GIGAS: A set of microwave sensor arrays to detect molecular bremsstrahlung radiation from extensive air shower" *NIM A* **888**, 153-162

### Related Work
3. **Gorham, P.W. et al.** (2008) "Observations of microwave continuum emission from air shower plasmas" *Phys.Rev.D* **78**, 032007
4. **Beresnyak, A. et al.** (2005) "Radio emission from cosmic ray air showers" *Astropart.Phys.* **24**, 206-218

## Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create feature branch (`git checkout -b feature/improvement`)
3. Commit changes (`git commit -am 'Add improvement'`)
4. Push to branch (`git push origin feature/improvement`)
5. Create Pull Request

## License

This software is distributed under the GNU General Public License v3.0. See `LICENSE` file for details.

## Contact

- **Lead Developer**: Dr. Imen Al Samarai
- **Institution**: LPNHE, CNRS/IN2P3
- **Email**: imen.alsamarai@lpnhe.in2p3.fr (now imen.alsamarai@gmail.com)

---

*This simulation represents pioneering work in radio detection of cosmic ray air showers, opening new possibilities for 24/7 UHECR observations with unprecedented duty cycle and cost-effectiveness.*

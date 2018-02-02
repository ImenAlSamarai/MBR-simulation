%module(package="mwoffline") common

// Provides typemaps for converting between STL and Python structures
%include "std_string.i"
%include "std_vector.i"
%include "std_list.i"
%include "std_pair.i"
%include "std_map.i"

%{
// Do not warn about unused variables
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#include <DllImport.h>
#include <Rtypes.h>

#include <TVector3.h>
%}

// Provide typemaps for ROOT datatypes and the ClassDef macro. We use import
// here because we don't want to wrap anything from these headers.
%import <DllImport.h>
%import <Rtypes.h>

// Define TObject as empty class to silence warnings about unknown base class.
class TObject {};

// Ignore useless operator definitions in TVector3.h
%ignore operator + (const TVector3&, const TVector3&);
%ignore operator - (const TVector3&, const TVector3&);
%ignore operator * (const TVector3&, const TVector3&);
%ignore operator * (const TVector3&, Double_t);
%ignore operator * (Double_t, const TVector3&);
%ignore operator * (const TMatrix&, const TVector3&);
%ignore TVector3::operator ();
%ignore TVector3::operator () (int);
%ignore TVector3::operator [] (int);
%ignore TVector3::operator [] (int) const;
%ignore TVector3::operator = (const TVector3&);

// Wrap commonly used ROOT classes
%include <TVector3.h>

// Instantiate commonly used templates so that they will be wrapped correctly
%template(DoubleTVector) std::vector<Double_t>;
%template(TVector3Vector) std::vector<TVector3>;
%template(DoubleVectorDoubleVectorPair) std::pair< std::vector<double>,
                                                   std::vector<double> >;

// Wrap additional operators of TVector3
%extend TVector3 {
  TVector3 __add__(const TVector3& b) {
    return *$self + b;
  }

  TVector3 __sub__(const TVector3& b) {
    return *$self - b;
  }

  TVector3 __mul__(const double& d) {
    return d * *$self;
  }

  TVector3 __rmul__(const double& d) {
    return d * *$self;
  }

  TVector3 __div__(const double& d) {
    return (1. / d) * *$self;
  }
}


%extend std::vector<Double_t> {
  PyObject* get_bytearray() {
    size_t nbytes = $self->size() * sizeof(Double_t);

    PyObject* array = PyByteArray_FromStringAndSize(NULL, nbytes);
    memcpy(((PyByteArrayObject*) array)->ob_bytes,
           &((*$self)[0]),
           nbytes);

    return array;
  }

  %pythoncode
  {
    def get_array(self):
      import numpy as np

      return np.frombuffer(self.get_bytearray(),
                           dtype=np.double)
  }
}

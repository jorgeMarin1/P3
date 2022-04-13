/// @file

#include <iostream>
#include <math.h>
#include "pitch_analyzer.h"

using namespace std;

/// Name space of UPC
namespace upc {
  void PitchAnalyzer::autocorrelation(const vector<float> &x, vector<float> &r) const {
    /// \DONE Compute the autocorrelation r[l]

    // Solo calcular el rango de autocorrelacion que se vaya a usar
    for (unsigned int l = npitch_min; l < npitch_max; ++l) {
        r[l] = 0.0f;

        for (unsigned int n = l; n < x.size(); n++) {
            r[l] += x[n]*x[n-l];
        }

        r[l] /= x.size();
    }

    // Como solo se calcula un rango de valores para la autocorrelación, podria
    // ser que r[0] y r[1] no se calcularan. Por esta razón estos valores se
    // calculan a parte.
    if (npitch_min > 0) {
      for (unsigned int l = 0; l <= 1; l++) {
        r[l] = 0.0f;
        for (unsigned int n = l; n < x.size(); n++) {
          r[l] += x[n]*x[n-l];
        }

        r[l] /= x.size();
      }
    }

    if (r[0] == 0.0F) //to avoid log() and divide zero 
      r[0] = 1e-10;
  }

  void PitchAnalyzer::set_window(Window win_type) {
    if (frameLen == 0)
      return;

    window.resize(frameLen);

    switch (win_type) {
    case HAMMING:
      /// \DONE Implement the Hamming window
      static const float alpha = 25.0f / 46.0f;
      static const float beta = 0.5f * (1 - alpha);

      for (unsigned int i = 0; i < frameLen; i++) {
        window[i] = alpha - 2*beta*cos(2*M_PI*i / (frameLen - 1));
      }

      break;
    case RECT:
    default:
      window.assign(frameLen, 1);
    }
  }

  void PitchAnalyzer::set_f0_range(float min_F0, float max_F0) {
    npitch_min = (unsigned int) samplingFreq/max_F0;
    if (npitch_min < 2)
      npitch_min = 2;  // samplingFreq/2

    npitch_max = 1 + (unsigned int) samplingFreq/min_F0;

    //frameLen should include at least 2*T0
    if (npitch_max > frameLen/2)
      npitch_max = frameLen/2;
  }

  bool PitchAnalyzer::unvoiced(float pot, float r1norm, float rmaxnorm, float zcr) const {
    //                unvoiced(pot,       r[1]/r[0],    r[lag]/r[0]),   zcr
    /// \DONE Implement a rule to decide whether the sound is voiced or not.
    /// * You can use the standard features (pot, r1norm, rmaxnorm),
    ///   or compute and use other ones.
    return pot < p_umaxpot || r1norm < n_umaxr1 || rmaxnorm < m_umaxnorm;
  }

  inline int PitchAnalyzer::sign(float x) const {
    return (x > 0) ? 1 : ((x < 0) ? -1 : 0);
  }

  float PitchAnalyzer::compute_zcr(const vector<float> &x, float fm) const {
    const float scaling = 0.5f * fm / (x.size() - 1);
    float zcr = 0.0f;

    for (unsigned int i = 1; i < x.size(); i++) {
      if (sign(x[i]) != sign(x[i - 1])) {
        zcr++;
      }
	  }

    return scaling*zcr;
  }

  float PitchAnalyzer::compute_pitch(vector<float> & x) const {
    if (x.size() != frameLen)
      return -1.0F;

    //Window input frame
    for (unsigned int i=0; i<x.size(); ++i)
      x[i] *= window[i];
    
    /// \DONE
    /// Find the lag of the maximum value of the autocorrelation away from the origin.<br>
    /// Choices to set the minimum value of the lag are:
    ///    - The first negative value of the autocorrelation.
    ///    - The lag corresponding to the maximum value of the pitch. **[This]**
    ///    
    /// In either case, the lag should not exceed that of the minimum value of the pitch.

    //Compute correlation
    vector<float> r(npitch_max);
    autocorrelation(x, r);

    vector<float>::const_iterator iR = r.begin(), iRMax = iR;

    for (iR = iRMax = r.begin() + npitch_min; iR < r.begin() + npitch_max; iR++) {
        if (*iR > *iRMax) {
            iRMax = iR;
        }
    }

    // Compute features
    unsigned int lag = iRMax - r.begin();
    float pot = 10 * log10(r[0]);
    float zcr = compute_zcr(x, samplingFreq);

    //You can print these (and other) features, look at them using wavesurfer
    //Based on that, implement a rule for unvoiced
    //change to #if 1 and compile
#if 0
    if (r[0] > 0.0F)
      cout << pot << '\t' << r[1]/r[0] << '\t' << r[lag]/r[0] << endl;
#endif
    
    if (unvoiced(pot, r[1]/r[0], r[lag]/r[0], zcr))
      return 0;
    else
      return (float) samplingFreq/(float) lag;
  }
}

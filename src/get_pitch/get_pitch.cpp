/// @file

#include <iostream>
#include <fstream>
#include <string.h>
#include <errno.h>

#include "wavfile_mono.h"
#include "pitch_analyzer.h"

#include "docopt.h"

#define FRAME_LEN   0.030 /* 30 ms. */
#define FRAME_SHIFT 0.015 /* 15 ms. */

using namespace std;
using namespace upc;

static const char USAGE[] = R"(
get_pitch - Pitch Estimator 

Usage:
    get_pitch [options] <input-wav> <output-txt>
    get_pitch (-h | --help)
    get_pitch --version

Options:
    -m FLOAT, --umaxnorm FLOAT      Umbral del máximo de la autocorrelación normalizada [default: 0.39]
    -n FLOAT, --umaxr1 FLOAT        Umbral de r1norm [default: 0.49]
    -p FLOAT, --umaxpot FLOAT       Umbral potencia [default: -46]
    -L INT, --median-length INT     Longitud del filtro de mediana [default: 3]
    -c FLOAT, --cc-height FLOAT     Altura del center clipping [default: 0.0]
    -H, --hamming                   Use the Hamming window
    -h, --help  Show this screen
    --version   Show the version of the project

Arguments:
    input-wav   Wave file with the audio signal
    output-txt  Output file: ASCII file with the result of the estimation:
                    - One line per frame with the estimated f0
                    - If considered unvoiced, f0 must be set to f0 = 0
)";

int main(int argc, const char *argv[]) {
    /// \TODO 
    ///  Modify the program syntax and the call to **docopt()** in order to
    ///  add options and arguments to the program.
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
        {argv + 1, argv + argc},    // array of arguments, without the program name
        true,    // show help if requested
        "2.0");  // version string

    std::string input_wav = args["<input-wav>"].asString();
    std::string output_txt = args["<output-txt>"].asString();
    float umaxnorm = stof(args["--umaxnorm"].asString());
    float umaxr1 = stof(args["--umaxr1"].asString());
    float umaxpot = stof(args["--umaxpot"].asString());
    int median_length = stoi(args["--median-length"].asString());
    float cc_height = stof(args["--cc-height"].asString());
    // Clampe cc_height between 0 and 1
    cc_height = cc_height < 0.0f ? 0.0f : cc_height;
    cc_height = cc_height > 1.0f ? 1.0f : cc_height;
    bool use_hamming = args["--hamming"].asBool();

  // Read input sound file
  unsigned int rate;
  vector<float> x;
  if (readwav_mono(input_wav, rate, x) != 0) {
    cerr << "Error reading input file " << input_wav << " (" << strerror(errno) << ")\n";
    return -2;
  }

  int n_len = rate * FRAME_LEN;
  int n_shift = rate * FRAME_SHIFT;

    // Define analyzer
    PitchAnalyzer::Window window;
    if (use_hamming) {
        window = PitchAnalyzer::Window::HAMMING;
    } else {
        window = PitchAnalyzer::Window::RECT;
    }
    PitchAnalyzer analyzer(n_len, rate, umaxnorm, umaxr1, umaxpot, window, MIN_F0, MAX_F0);
    
  /// \DONE
  /// Preprocess the input signal in order to ease pitch estimation. For instance,
  /// **central-clipping** or low pass filtering may be used.
  
  /// Se calcula el umbral del center clipping. Para ello iteramos y buscamos el máximo.
  /// El umbral será igual al 30% del máxima de la señal.
    float Xth = 0, max = 0;

    vector<float>::iterator iX;

    for(iX= x.begin(); iX < x.end(); iX++){
        if(*iX > max){
            max = *iX;
        }
    }

    Xth = cc_height * max;

    for(iX= x.begin(); iX < x.end(); iX++) {
        if(*iX > Xth) {
            *iX = *iX - Xth;
        } else {
            if(*iX < -1*Xth) {
                *iX = *iX + Xth;
            } else {
                *iX = 0;
            }
        }
    }

    // Iterate for each frame and save values in f0 vector
    vector<float> f0;
    for (iX = x.begin(); iX + n_len < x.end(); iX = iX + n_shift) {
        float f = analyzer(iX, iX + n_len);
        f0.push_back(f);
    }

    /// \DONE
    /// Postprocess the estimation in order to supress errors. For instance, a
    /// **median filter** or time-warping may be used.

    // Aplicar filtro de mediana de longitud median_length:
    // Se crea un vector result donde se iran guardando los resultados, para
    // que el filtro no se recursivo. Una vez calculado se copia el vector
    // result a f0.
    const int after = (median_length - 1) / 2;
    const int before = (median_length - 1) / 2;

    vector<float> result(f0);
    vector<float> tmp(median_length);
    vector<float>::iterator it_res;

    int i = before - 1;
    for (it_res = result.begin() + before - 1; it_res < result.end() - after; it_res++) {
        copy(it_res - before, it_res + after + 1, tmp.begin());
        sort(tmp.begin(), tmp.end());

        f0[i] = tmp[(median_length - 1) / 2];

        i++;
    }

  // Write f0 contour into the output file
  ofstream os(output_txt);
  if (!os.good()) {
    cerr << "Error reading output file " << output_txt << " (" << strerror(errno) << ")\n";
    return -3;
  }

  os << 0 << '\n'; //pitch at t=0
  for (iX = f0.begin(); iX != f0.end(); ++iX) 
    os << *iX << '\n';
  os << 0 << '\n';//pitch at t=Dur

  return 0;
}

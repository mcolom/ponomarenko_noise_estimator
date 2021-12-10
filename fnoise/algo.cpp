/*
 *
 * This file is part of the fnoise algorithm.
 *
 * Copyright(c) 2011 Miguel Colom.
 * http://mcolom.info
 *
 * This file may be licensed under the terms of of the
 * GNU General Public License Version 2 (the ``GPL'').
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the GPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the GPL along with this
 * program. If not, go to http://www.gnu.org/licenses/gpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <ctime>
#ifdef _OPENMP
#include <omp.h>
#endif
//
#include "../framework/CImage.h"
#include "../framework/libparser.h"

//! Gets the upper bound to truncate the values, according to the number of bits
/*!
  \param bits Number of bits
  \return Truncation value
*/
int get_trunc_value(int bits) {
  return pow(2, bits) - 1;
}

//! Adds uniform noise
/*!
  \param u Input image
  \param v Output image
  \param std Standard deviation of the noise
  \param truncate Truncate values outside range [0 and 255].
  \param size Length of the input and output matrices
*/
void add_noise(float *u, float *v,
               float std, bool truncate, int size, int bits_output) {
    int v_max = get_trunc_value(bits_output);
    //
    #ifdef _OPENMP
    #pragma omp parallel for
    #endif
    for (int i = 0; i < size; i++) {
        double a = drand48();
        double b = drand48();
        double z = (double)(std)*sqrt(-2.0*log(a))*cos(2.0*M_PI*b);

        v[i] =  u[i] + (float) z;
        if (truncate) {
                if (v[i] < 0) {
                        v[i] = 0;
                }
                else {
                        if (v[i] > v_max) {
                                v[i] = v_max;
                        }
                }
        } // if truncate
    } // for i
}

//! Adds noise with variance V = A + Bu, where u is the input image luminance.
/*!
  \param u Input image
  \param v Output image
  \param A Parameter of the noise variance V = A + Bu
  \param B Parameter of the noise variance V = A + Bu
  \param truncate Truncate values outside range [0 and 255].
  \param size Length of the input and output matrices
*/
void add_noise_affine(float *u, float *v,
               float A, float B, bool truncate, int size, int bits_output) {
    //
    int v_max = get_trunc_value(bits_output);
    //
    #ifdef _OPENMP
    #pragma omp parallel for
    #endif
    for (int i = 0; i < size; i++) {
        float std = sqrt(A + B*u[i]);

        double a = drand48();
        double b = drand48();
        double z = (double)(std)*sqrt(-2.0*log(a))*cos(2.0*M_PI*b);

        v[i] =  u[i] + (float) z;
        if (truncate) {
                if (v[i] < 0) {
                        v[i] = 0;
                }
                else {
                        if (v[i] > v_max) {
                                v[i] = v_max;
                        }
                }
        } // if truncate
    } // for i
}

//! Copy the input image to the output image
/*!
  \param *in input image
  \param *out outputimage
  \param N number of pixels
*/
void copy_image(float *in, float *out, int N) {
  for (int i = 0; i < N; i++)
    out[i] = in[i];
}


//! Add uniform noise algorithm.
/*!
  \param *framework Framework
  \param argc Number of arguments of the program
  \param **argv Arguments of the program
*/
void algorithm(int argc, char **argv) {
  vector <OptStruct *> options;
  vector <ParStruct *> parameters;

  OptStruct og = {"g:", 0,  NULL, NULL, "noise standard deviation"};
  options.push_back(&og);
  OptStruct oA = {"A:", 0,  NULL, NULL, "noise standard deviation"};
  options.push_back(&oA);
  OptStruct oB = {"B:", 0,  NULL, NULL, "noise standard deviation"};
  options.push_back(&oB);
  OptStruct ob = {"b:", 0,  "8", "8", "Bits per channel in the output"};
  options.push_back(&ob);
  OptStruct ot = {"t", 0, NULL, NULL, "Flag to truncate between [0, 255 or 65535]"};
  options.push_back(&ot);
  OptStruct os = {"s:", 0,  "0", "0", "Seed used to initialize the random number generator. 0=Random"};
  options.push_back(&os);

  //
  ParStruct pinput = {"image", NULL, "image"}; parameters.push_back(&pinput);
  ParStruct pout = {"out", NULL, "output file"}; parameters.push_back(&pout);

  if (!parsecmdline("fnoise","adds white gaussian noise",
                    argc, argv, options, parameters)) {
    printf("\n");
    printf("Under license GNU GPL by Miguel Colom, 2012\n");
    printf("http://mcolom.info/\n");
    printf("\n");    
    exit(-1);
  }

  // Uniform or affine noise
  bool uniform = og.flag;
  bool affine = oA.flag && oB.flag;

  // Check if the selection is right
  if (!(uniform || affine)) {
    PRINT_ERROR("Error: neither uniform nor affine noise specified.\n");
    exit(-1);
  }

  if (uniform && affine) {
    PRINT_ERROR("Error: both uniform and affine noise specified.\n");
    exit(-1);
  }

  // Parameters
  bool truncate = ot.flag;
  int bits_output = atoi(ob.value);
  long int seed = atoi(os.value);

  // Initialize then random number generator
  if (seed == 0)
    srand48((long int)time(NULL) + (long int)clock());
  else
    srand48(seed);

  // Load input image
  CImage input;
  input.load((char*)pinput.value);

  // Create output
  CImage *output = new CImage(input.get_width(), input.get_height(),
                              input.get_bits_per_channel(),
                              input.get_num_channels());

  int N = input.get_width() * input.get_height();
  
  #ifdef _OPENMP
  omp_set_num_threads(omp_get_num_procs());
  #endif

  if (uniform) {
    float sigma = atof(og.value);
      // Add noise to each channel
    #ifdef _OPENMP
    #pragma omp parallel for
    #endif
    for (int ch = 0; ch < input.get_num_channels(); ch++) {
      float *in = input.get_channel(ch);
      float *out = output->get_channel(ch);
      //
      if (sigma > 0)
        add_noise(in, out, sigma, truncate, N, bits_output);
      else
        copy_image(in, out, N);
    }
  } else { // Affine
    float A = atof(oA.value);
    float B = atof(oB.value);
    // Add noise to each channel

    #ifdef _OPENMP
    #pragma omp parallel for
    #endif
    for (int ch = 0; ch < input.get_num_channels(); ch++) {
      float *in = input.get_channel(ch);
      float *out = output->get_channel(ch);
      //
      if (A > 0 || B > 0)
        add_noise_affine(in, out, A, B, truncate, N, bits_output);
      else
        copy_image(in, out, N);
    }
  }

  // Save output
  output->save((char*)pout.value, bits_output);
  delete output;
}

int main(int argc, char **argv) {
  CFramework *fw = CFramework::get_framework();
  //try {
    algorithm(argc, argv);
    delete fw;
  //}
  /*catch(...) {    
    printf("An exception raised while running the algorithm.\n");
    printf("Backtrace:\n");
    fw->print_backtrace(stdout);
    // [ToDo]: call user handler for the exception
    exit(-1);
  }*/
}

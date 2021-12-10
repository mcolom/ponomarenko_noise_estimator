/*
 *
 * This file is part of the subscale algorithm.
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

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <vector>
//
#include "../framework/CImage.h"
#include "../framework/libparser.h"

//! Reads a block of the image given its coordinates
/*!
  \param *block Buffer where the pixels will be written
  \param *in Input image
  \param x X coordinate
  \param y Y coordinate
  \param Nx Width of the image
  \param Ny Height of the image
*/
inline void get_block(float *block, float *in,
                      int x, int y,
                      int Nx, int w) {
  for (int j = 0; j < w; j++)
      for (int i = 0; i < w; i++)
          block[w*j + i] = in[Nx*(y+j) + (x+i)];
}

//! Compute the mean of a block
/*!
  \param *block Block to compute the mean from
  \param w Width of the block
  \return The mean of the pixels inside the block
*/
inline float mean_block(float *block, int w) {
  float sum = 0.0;
  for (int j = 0; j < w; j++)
      for (int i = 0; i < w; i++)
          sum += block[w*j + i];
  //
  return sum / (w*w);
}

//! Add subscale algorithm.
/*!
  \param *framework Framework
  \param argc Number of arguments of the program
  \param **argv Arguments of the program
*/
void algorithm(int argc, char **argv) {
  vector <OptStruct *> options;
  vector <ParStruct *> parameters;

  OptStruct os = {"s:", 0,  "2", NULL, "block side"};
  options.push_back(&os);
  //
  ParStruct pinput = {"image", NULL, "image"}; parameters.push_back(&pinput);
  ParStruct pout = {"out", NULL, "output file"}; parameters.push_back(&pout);

  if (!parsecmdline("subscale", "subscale of image",
                    argc, argv, options, parameters)) {
    printf("\n");
    printf("Under license GNU GPL by Miguel Colom, 2012\n");
    printf("http://mcolom.perso.math.cnrs.fr/\n");
    printf("\n");    
    exit(-1);
  }

  // Parameters
  int w = atoi(os.value);

  // Load input image
  CImage input;
  input.load((char*)pinput.value);

  // Input image properties
  int bits = input.get_bits_per_channel();
  int num_channels = input.get_num_channels();
  int Nx = input.get_width();
  int Ny = input.get_height();

  // Output image size
  int oNx = (int)(floor(Nx / w));
  int oNy = (int)(floor(Ny / w));

  // Create output
  CImage *output = new CImage(oNx, oNy,
                              bits,
                              num_channels);

  // Change each block of w*w pixels by its mean to down-scale the input image
  float *block = new float[w*w];
  
  for (int ch = 0; ch < num_channels; ch++) {      
      float *in  = input.get_channel(ch);
      float *out = output->get_channel(ch);

      for (int oy = 0; oy < oNy; oy ++) {
          for (int ox = 0; ox < oNx; ox ++) {
              get_block(block, in,
                        ox*w, oy*w,
                        Nx, w);

              float block_mean = mean_block(block, w);

              out[oNx*oy+ox] = block_mean;
          }
      }
  } // for ch

  delete[] block;  

  // Save output
  output->save((char*)pout.value, input.get_bits_per_channel());
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

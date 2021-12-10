# Ponomarenko noise estimator
![estimation](https://ipolcore.ipol.im/api/core/shared_folder/run/45/8E2EF701685E11C17818209A0FCF3A58/curve_s0.png)</p>

This is the source code corresponding to the "Analysis and extension of the Ponomarenko et al method, estimating a noise curve from a single image" article published in the Image Processing On Line (IPOL) journal.

https://www.ipol.im/pub/art/2013/45/  
DOI: 10.5201/ipol.2013.45

The sources include the program to estimate signal-dependent noise and also the additional tools needed to run the demo:
- "fnoise" to add uniform or signal dependent noise,
- "subscale" to create down-scales by averaging groups of pixels.

To compile each program the 'make' tool can be invoked at each of the three directories:
- fnoise
- ponomarenko
- subscale

To make the source code much more understandable, the auxiliar functions that deal with memory management, image reading, garbage collecting, etc. has been isolated in the "framework" directory.
The file "algo.cpp" contains only that part of the code that is relevant for the noise estimation algorithm.
All these programs are standalone and therefore none of them need the others to run.

The programs needs the FFTW3 library to be run and optionally the openMP library.
Complete list of libraries (g++ command line): -lpng -ltiff -lm -lfftw3f -lfftw3f_threads -lgomp.

The ponomarenko program is able to read 8/16 bits PNG files and also float images encoded with its own format.

The usage of the program and the default values are the following:

usage: ponomarenko [-w w] [-p p] [-r] [-b b] [-g g] [-m m]  input 
	-w  w	 Block side (Default: 8)
	-p  p	 Percentile (Default: 0.005)
	-r	 Flag to remove equal pixels 
	-b  b	 Number of bins (Default: 0)
	-g  g	 Filter curve iterations (Default: 2)
	-m  m	 Mean computation method (Default: 2)
	input	 input file

Automatic settings:
- Setting the percentile to 0 means that the original loop of the Ponomarenko algorithm will be used.
- Setting the number of bins to 0 means that 42000 samples/bin will be used automatically.

Example of use:

```
miguel@pringle:~/ponomarenko$ ./ponomarenko -m1 lena.png
97.556854  20.464422  65.181114  2.571376  3.398507  3.328110  
161.822037  65.409187  77.526772  2.248434  3.205869  3.068921  
176.508362  92.413513  95.091988  2.091094  2.902737  3.781942  
208.454453  108.978432  110.369133  1.510017  2.607002  4.128088  
221.517090  134.267456  125.007042  1.280891  2.381180  4.175092  
240.984161  203.753891  181.479172  0.807139  1.327316  3.185435
```

The first three columns are the means of the noise curves. The last three the corresponding standard deviations. This format is directly compatible with gnuplot.

The source code of all the C/C++ programs include their comments in such a way that automatic documentation can be generated with the Doxygen tool.

http://www.ipol.im  
Image Processing On Line

Copyright (c) 2011 by Miguel Colom.  
http://mcolom.info/  

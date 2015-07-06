                               multirate_algs 1.3

multirate_algs is a free and open collection of algorithms for implementing
multirate Finite Impulse Response (FIR) filters in C. It is provided with
ScopeFIR, which is a comprehensive FIR filter design program for Windows.  (See
http://www.iowegian.com/scopefir.htm for more information.)  ScopeFIR was used
to design the FIR filters used to test multirate_algs.

multirate_algs consists of the following files:

    decim.c/.h          Decimating FIR: reduces the sampling rate by an integer
                        factor
    interp.c/.h         Interpolating FIR: increases the sampling rate by an
                        integer factor
    resamp.c/.h   	Resampling FIR, to change sampling rate by an integral
                        ratio
    interp21.inc/.sfp   Coefficients and ScopeFIR project file for an
                        interpolate-by-21 FIR
    interp25.inc/.sfp	Coefficients and ScopeFIR project file for an
                        interpolate-by-25 FIR

    mr_test.c           Multirate test program
    mr_test.sln/.vcproj Microsoft Visual C++ .net project files for the test
                        program
    makefile            GCC Make file for the test program

multirate_algs is provided under terms of the "Wide Open License".  See the
".c" files or http://dspguru.com/wol.htm for more details.

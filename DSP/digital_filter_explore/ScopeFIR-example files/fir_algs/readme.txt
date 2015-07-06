                                 fir_algs 1.1

fir_algs is a free and open collection of Finite Impulse Response (FIR) filter
algorithms written in C.  It consists of "fir_algs.c" plus project files to
compile it using Microsoft Visual C++ .net.

fir_algs.c contains several FIR filter functions and a comprehensive test
program.  It is provided in the distribution file of ScopeFIR, which is a
comprehensive FIR filter design program for Windows.
(See http://www.iowegian.com/scopefir.htm for more information.)

fir_algs includes the following FIR functions intended to help you understand
the basic FIR algorithm and some common hardware implementations:

    fir_basic	    the FIR algorithm implemented in the most straight-forward
                    way.
    fir_circular    the FIR algorithm implemented as many DSP processors do,
                    via a circular buffer.
    fir_shuffle     the FIR algorithm implemented as TI's older fixed-point
                    processors do, by shuffling the data down at each iteration.

The functions above are more for illustration than for actual use because they
aren't particularly efficient without special hardware.  When written in C for
a general-purpose processor (such as a PC) the FIR algorithm can be optimized
for speed by reducing the movement of data.  In that vein, fir_algs includes
the following additional functions to illustrate various strategies for
speeding the FIR filter algorithm:

    fir_split      splits the FIR calculation into two parts to avoid data
                   movement
    fir_double_z   uses a double-sized delay line to avoid data movement
    fir_double_h   uses a double-sized coefficient set to avoid data movement.
                   This is the speediest of all the functions included in
                   fir_algs, which makes it the best choice for use in real C
                   implementations.

fir_algs is provided under terms of the "Wide Open License".  See fir_algs.c or
http://dspguru.com/wol.htm for more details.


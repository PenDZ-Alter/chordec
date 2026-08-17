#ifndef MATH_H
#define MATH_H

#include <iostream>
#include <vector>
#include <complex>

#include "params.h"

/**
 * Fast-Fourier Transform
 * @param <vector> size of frequency
 */
void fft(std::vector<Complex>& x);

#endif
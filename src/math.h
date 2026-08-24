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

/**
 * Calculate Cosine Similarity between two vectors
 * @param chroma Chroma vector (size 12)
 * @param templateProfile Template vector (size 12)
 * @return Cosine Similarity value
 */
double calculateCosineSimilarity(const std::vector<double>& chroma, const std::vector<double>& templateProfile);

#endif
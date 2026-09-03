#include "ca_math.h"

void fft(std::vector<Complex>& x) 
{
    size_t N = x.size();
    if (N <= 1) return;

    std::vector<Complex> even(N / 2), odd(N / 2);
    for (size_t i = 0; i < N / 2; ++i) {
        even[i] = x[i * 2];
        odd[i]  = x[i * 2 + 1];
    }

    fft(even);
    fft(odd);

    for (size_t k = 0; k < N / 2; ++k) {
        Complex t = std::polar(1.0, -2 * PI * k / N) * odd[k];
        x[k]       = even[k] + t;
        x[k + N/2] = even[k] - t;
    }
}

double calculateCosineSimilarity(const std::vector<double>& chroma, const std::vector<double>& templateProfile) 
{
    double dotProduct = 0.0;
    double normChroma = 0.0;
    double normTemplate = 0.0;

    for (int i = 0; i < 12; ++i) {
        dotProduct += chroma[i] * templateProfile[i];
        normChroma += chroma[i] * chroma[i];
        normTemplate += templateProfile[i] * templateProfile[i];
    }

    if (normChroma == 0.0 || normTemplate == 0.0) return 0.0;
    return dotProduct / (std::sqrt(normChroma) * std::sqrt(normTemplate));
}
//
// Created by Devyn Oh on 6/19/25.
//

#include "dft.h"
#include <iostream>
#include <cmath>

using Cmplx = std::complex<double>;

std::vector<FourierComponent> fourier(const std::vector<Cmplx>& path, const int depth) {


    const int N = path.size();
    constexpr double TAU = 2.0 * M_PI;
    std::vector<FourierComponent> dft;

    const int mid = depth >> 1;
    const int minf = -mid; // The lowest frequency calculated
    const int maxf = depth - mid; // The highest frequency calculated

    for (int f = minf; f <= maxf; ++f) {
        // Define the function to integrate
        auto func = [&](double t) -> Cmplx {
            int i = std::round(t * (N - 1));
            if (i < 0) i = 0;
            if (i >= N) i = N - 1; // Collapses the domain to stop overflows
            Cmplx expTerm = std::polar(1.0, -f * TAU * t);  // same as Complex({abs:1, arg:...})
            // This is the same as e ^ -tau n t
            // At every time t, find the point that is closest to t and add the angle of t to that point
            return path[i] * expTerm;
        };

        Cmplx Cn = integrateC(func, 0.0, 1.0);

        dft.push_back({
            f,
            std::abs(Cn),
            std::arg(Cn)
        });

        double progress = 100.0 * (f - minf) / (maxf - minf);
        //std::cout << "progress " << progress << "%" << std::endl;
    }

    return dft;
}



Cmplx integrateC(std::function<Cmplx(double)> func, double a, double b) {
    // simple trapezoidal implementation:
    const int steps = 1000; // crude fixed-step version
    double h = (b - a) / steps;
    Cmplx sum = 0.5 * (func(a) + func(b));

    // calculating function
    // sum from t=1 to steps of f(a+th) * e^(-2pi in(a+th))
    // where f is the function, h is (b-a)/steps, and n is the partial being calculated
    for (int i = 1; i < steps; ++i) {
        double x = a + i * h;
        sum += func(x);
    }

    return sum * h;
}



// double f(double x) {
//     return 1/x;
// }

void printFourierComponents(const std::vector<FourierComponent>& components) {
    for (const auto& comp : components) {
        std::cout << "Freq: " << comp.freq
                  << ", Amp: " << comp.amp
                  << ", Phase: " << comp.phase << '\n';
    }
}
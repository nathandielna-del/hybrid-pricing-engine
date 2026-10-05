#include "pricer/math/NormalDistribution.h"

#include <cmath>
#include <numbers>

namespace pricer {
    constexpr double invSqrtTwoPi = std::numbers::inv_sqrtpi / std::numbers::sqrt2;
    
    double normalPdf(double x) noexcept {
        return invSqrtTwoPi * std::exp(-(x*x)/2.0);
    }

    double normalCdf(double x) noexcept {
        return 0.5 * std::erfc(-x/std::numbers::sqrt2);
    }

} // namespace pricer
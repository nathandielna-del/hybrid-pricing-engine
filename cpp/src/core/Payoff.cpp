#include "pricer/core/Payoff.h"

#include <algorithm>

namespace pricer {

double callPayoff(double spot, double strike) {
    return std::max(spot - strike, 0.0);
}

double putPayoff(double spot, double strike) {
    return std::max(strike - spot, 0.0);
}

}  // namespace pricer
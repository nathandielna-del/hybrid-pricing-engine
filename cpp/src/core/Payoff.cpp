#include "pricer/core/Payoff.h"

#include <algorithm>
#include <stdexcept>

namespace pricer {

double callPayoff(double spot, double strike) {
    return std::max(spot - strike, 0.0);
}

double putPayoff(double spot, double strike) {
    return std::max(strike - spot, 0.0);
}

namespace {

// Returns the strike if valid, throws otherwise.
// Written as a function returning a value so it can be used in the initializer list.
double validatedStrike(double strike) {
    if (!(strike > 0.0)) {  // also rejects NaN, unlike (strike <= 0.0)
        throw std::invalid_argument("Payoff: strike must be strictly positive.");
    }
    return strike;
}

}  // namespace

CallPayoff::CallPayoff(double strike) : strike_{validatedStrike(strike)} {}

double CallPayoff::operator()(double spotAtMaturity) const noexcept {
    return callPayoff(spotAtMaturity, strike_);
}

double CallPayoff::strike() const noexcept {
    return strike_;
}

PutPayoff::PutPayoff(double strike) : strike_{validatedStrike(strike)} {}

double PutPayoff::operator()(double spotAtMaturity) const noexcept {
    return putPayoff(spotAtMaturity, strike_);
}

double PutPayoff::strike() const noexcept {
    return strike_;
}

DigitalCallPayoff::DigitalCallPayoff(double strike) : strike_{validatedStrike(strike)} {}

double DigitalCallPayoff::operator()(double spotAtMaturity) const {
    return spotAtMaturity > strike_ ? 1.0 : 0.0;
}
double DigitalCallPayoff::strike() const noexcept {
    return strike_;
}

}  // namespace pricer
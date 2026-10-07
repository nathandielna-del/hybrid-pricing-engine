#include "pricer/core/VanillaOption.h"

#include <memory>
#include <stdexcept>
#include <utility>

namespace pricer {

namespace {

// Returns the payoff if it points to an object, throws if it is empty.
std::shared_ptr<const Payoff> validatedPayoff(std::shared_ptr<const Payoff> payoff) {
    if (!payoff) {
        throw std::invalid_argument("VanillaOption: payoff must not be null.");
    }
    return payoff;  // moved out automatically, no copy
}

double validatedMaturity(double maturity) {
    if (!(maturity > 0.0)) {  // also rejects NaN
        throw std::invalid_argument("VanillaOption: maturity must be strictly positive.");
    }
    return maturity;
}

}  // namespace

VanillaOption::VanillaOption(std::shared_ptr<const Payoff> payoff, double maturity)
    : payoff_{validatedPayoff(std::move(payoff))},
      maturity_{validatedMaturity(maturity)} {}

const Payoff& VanillaOption::payoff() const noexcept {
    return *payoff_;  // never null: guaranteed by the constructor
}

double VanillaOption::maturity() const noexcept {
    return maturity_;
}

double VanillaOption::payoffAt(double spotAtMaturity) const {
    return (*payoff_)(spotAtMaturity);
}

}  // namespace pricer
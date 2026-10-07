#pragma once

#include "pricer/core/Payoff.h"

#include <memory>

namespace pricer {

// A European vanilla option: what is paid (payoff) and when (maturity, in years).
// Market data (spot, rate, volatility) deliberately does not belong here.
class VanillaOption {
public:
    // Throws std::invalid_argument if payoff is null or maturity is not strictly positive.
    VanillaOption(std::shared_ptr<const Payoff> payoff, double maturity);

    [[nodiscard]] const Payoff& payoff() const noexcept;
    [[nodiscard]] double maturity() const noexcept;

    // Convenience: h(S_T) for this option.
    [[nodiscard]] double payoffAt(double spotAtMaturity) const;

private:
    std::shared_ptr<const Payoff> payoff_;
    double maturity_;
};

}  // namespace pricer
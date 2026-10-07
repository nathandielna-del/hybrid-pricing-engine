#pragma once

#include "pricer/core/VanillaOption.h"

namespace pricer {

// Abstract pricing method: computes the present value of an option.
class PricingEngine {
public:
    virtual ~PricingEngine() = default;

    PricingEngine(const PricingEngine&) = delete;
    PricingEngine& operator=(const PricingEngine&) = delete;
    PricingEngine(PricingEngine&&) = delete;
    PricingEngine& operator=(PricingEngine&&) = delete;

    [[nodiscard]] virtual double price(const VanillaOption& option) const = 0;

protected:
    PricingEngine() = default;
};

}  // namespace pricer
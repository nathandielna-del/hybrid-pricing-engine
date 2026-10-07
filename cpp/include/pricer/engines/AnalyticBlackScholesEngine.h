#pragma once

#include "pricer/engines/PricingEngine.h"
#include "pricer/models/BlackScholes.h"
#include "pricer/models/BlackScholesModel.h"

namespace pricer {

// Closed-form Black-Scholes pricing. Supports CallPayoff and PutPayoff only.
class AnalyticBlackScholesEngine final : public PricingEngine {
public:
    explicit AnalyticBlackScholesEngine(const BlackScholesModel& model);

    // Throws std::invalid_argument if the payoff is neither a call nor a put.
    [[nodiscard]] double price(const VanillaOption& option) const override;
    [[nodiscard]] Greeks greeks(const VanillaOption& option) const;

private:
    BlackScholesModel model_;
};

}  // namespace pricer
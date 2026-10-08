#pragma once

#include <cstddef>
#include <cstdint>

#include "pricer/engines/MonteCarloResult.h"
#include "pricer/engines/PricingEngine.h"
#include "pricer/models/BlackScholesModel.h"

namespace pricer {

// Prices a European option by Monte-Carlo: simulates S_T exactly under Black-Scholes,
// averages the payoffs and discounts the average. Works with any Payoff.
// Each call restarts from the seed, so the same engine always returns the same result.
class MonteCarloEngine final : public PricingEngine {
public:
    // numberOfPaths must be at least 2 (the sample variance needs N - 1 > 0).
    MonteCarloEngine(const BlackScholesModel& model, std::size_t numberOfPaths, std::uint64_t seed);

    // Price only, as required by PricingEngine. Same value as simulate(option).price.
    [[nodiscard]] double price(const VanillaOption& option) const override;

    // Price together with its standard error and 95% confidence interval.
    [[nodiscard]] MonteCarloResult simulate(const VanillaOption& option) const;

    [[nodiscard]] std::size_t numberOfPaths() const noexcept;
    [[nodiscard]] std::uint64_t seed() const noexcept;

private:
    BlackScholesModel model_;
    std::size_t numberOfPaths_;
    std::uint64_t seed_;
};

}  // namespace pricer
#pragma once

#include <cstddef>
#include <cstdint>

#include "pricer/engines/PricingEngine.h"
#include "pricer/models/BlackScholesModel.h"

namespace pricer {

// Prices a European option by Monte-Carlo: simulates S_T exactly under Black-Scholes,
// averages the payoffs and discounts the average. Works with any Payoff.
// Each call to price() restarts from the seed, so the same engine always returns the same price.
class MonteCarloEngine final : public PricingEngine {
public:
    MonteCarloEngine(const BlackScholesModel& model, std::size_t numberOfPaths, std::uint64_t seed);

    [[nodiscard]] double price(const VanillaOption& option) const override;

    [[nodiscard]] std::size_t numberOfPaths() const noexcept;
    [[nodiscard]] std::uint64_t seed() const noexcept;

private:
    BlackScholesModel model_;
    std::size_t numberOfPaths_;
    std::uint64_t seed_;
};

}  // namespace pricer
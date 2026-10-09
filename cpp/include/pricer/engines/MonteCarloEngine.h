#pragma once

#include <cstddef>
#include <cstdint>

#include "pricer/engines/MonteCarloResult.h"
#include "pricer/engines/PricingEngine.h"
#include "pricer/models/BlackScholesModel.h"

namespace pricer {

// How each Monte-Carlo sample is built from a normal draw Z.
enum class SamplingScheme {
    Standard,    // one path per draw: sample = payoff(S_T(Z))
    Antithetic,  // two mirrored paths per draw: sample = (payoff(S_T(Z)) + payoff(S_T(-Z))) / 2
};

// Prices a European option by Monte-Carlo: simulates S_T exactly under Black-Scholes,
// averages the payoffs and discounts the average. Works with any Payoff.
// Each call restarts from the seed, so the same engine always returns the same result.
class MonteCarloEngine final : public PricingEngine {
public:
    // numberOfPaths is the number of samples, at least 2 (the sample variance needs N - 1 > 0).
    // With SamplingScheme::Antithetic each sample uses 2 paths, so the cost is doubled.
    MonteCarloEngine(const BlackScholesModel& model, std::size_t numberOfPaths, std::uint64_t seed,
                     SamplingScheme scheme = SamplingScheme::Standard);

    // Price only, as required by PricingEngine. Same value as simulate(option).price.
    [[nodiscard]] double price(const VanillaOption& option) const override;

    // Price together with its standard error and 95% confidence interval.
    [[nodiscard]] MonteCarloResult simulate(const VanillaOption& option) const;

    [[nodiscard]] std::size_t numberOfPaths() const noexcept;
    [[nodiscard]] std::uint64_t seed() const noexcept;
    [[nodiscard]] SamplingScheme scheme() const noexcept;

private:
    BlackScholesModel model_;
    std::size_t numberOfPaths_;
    std::uint64_t seed_;
    SamplingScheme scheme_;
};

}  // namespace pricer
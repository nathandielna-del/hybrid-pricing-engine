#include "pricer/engines/MonteCarloEngine.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "pricer/core/VanillaOption.h"
#include "pricer/math/NormalGenerator.h"

namespace pricer {

namespace {
std::size_t validatedNumberOfPaths(std::size_t numberOfPaths) {
    if (numberOfPaths < 2) {
        throw std::invalid_argument("MonteCarloEngine: number of paths must be at least 2");
    }
    return numberOfPaths;
}
}  // namespace

MonteCarloEngine::MonteCarloEngine(const BlackScholesModel& model, std::size_t numberOfPaths,
                                   std::uint64_t seed, SamplingScheme scheme)
    : model_{model},
      numberOfPaths_{validatedNumberOfPaths(numberOfPaths)},
      seed_{seed},
      scheme_{scheme} {}

std::size_t MonteCarloEngine::numberOfPaths() const noexcept {
    return numberOfPaths_;
}

std::uint64_t MonteCarloEngine::seed() const noexcept {
    return seed_;
}

SamplingScheme MonteCarloEngine::scheme() const noexcept {
    return scheme_;
}

double MonteCarloEngine::price(const VanillaOption& option) const {
    return simulate(option).price;
}

MonteCarloResult MonteCarloEngine::simulate(const VanillaOption& option) const {
    NormalGenerator generator{seed_};
    const double maturity = option.maturity();
    double payoffSum = 0.0;
    double payoffSquaresSum = 0.0;
    for (std::size_t i = 0; i < numberOfPaths_; ++i) {
        const double z = generator.next();
        const double payoff = scheme_ == SamplingScheme::Antithetic
                                  ? 0.5 * (option.payoffAt(model_.terminalSpot(maturity, z)) +
                                           option.payoffAt(model_.terminalSpot(maturity, -z)))
                                  : option.payoffAt(model_.terminalSpot(maturity, z));
        payoffSum += payoff;
        payoffSquaresSum += payoff * payoff;
    }
    const double n = static_cast<double>(numberOfPaths_);
    const double mean = payoffSum / n;
    const double variance = std::max(0.0, (payoffSquaresSum - n * mean * mean) / (n - 1.0));
    const double standardError = std::sqrt(variance / n);
    const double discountFactor = model_.discountFactor(maturity);
    return MonteCarloResult{.price = mean * discountFactor,
                            .standardError = standardError * discountFactor};
}

}  // namespace pricer
#include "pricer/engines/MonteCarloEngine.h"

#include <stdexcept>

#include "pricer/core/VanillaOption.h"
#include "pricer/math/NormalGenerator.h"

namespace pricer {

namespace {
std::size_t validatedNumberOfPaths(std::size_t numberOfPaths) {
    if (numberOfPaths == 0) {
        throw std::invalid_argument("MonteCarloEngine: number of paths must be striclty positive");
    }
    return numberOfPaths;
}
}  // namespace

MonteCarloEngine::MonteCarloEngine(const BlackScholesModel& model, std::size_t numberOfPaths,
                                   std::uint64_t seed)
    : model_{model}, numberOfPaths_{validatedNumberOfPaths(numberOfPaths)}, seed_{seed} {}

std::size_t MonteCarloEngine::numberOfPaths() const noexcept {
    return numberOfPaths_;
}

std::uint64_t MonteCarloEngine::seed() const noexcept {
    return seed_;
}

double MonteCarloEngine::price(const VanillaOption& option) const {
    NormalGenerator generator{seed_};
    const double maturity = option.maturity();
    double payoffSum = 0.0;
    for (std::size_t i = 0; i < numberOfPaths_; ++i) {
        payoffSum += option.payoffAt(model_.terminalSpot(maturity, generator.next()));
    }
    return model_.discountFactor(maturity) * payoffSum / static_cast<double>(numberOfPaths_);
}

}  // namespace pricer
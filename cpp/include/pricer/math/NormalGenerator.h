#pragma once

#include <cstdint>
#include <random>

namespace pricer {

// Reproducible source of independent standard normal draws Z ~ N(0, 1).
// The same seed always produces the same sequence of draws.
class NormalGenerator {
public:
    explicit NormalGenerator(std::uint64_t seed);

    // Returns the next draw. Not const: each call advances the internal state.
    [[nodiscard]] double next();

private:
    std::mt19937_64 engine_;
    std::normal_distribution<double> distribution_;
};

}  // namespace pricer
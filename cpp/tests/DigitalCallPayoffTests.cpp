#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>

#include <gtest/gtest.h>

#include "pricer/core/Payoff.h"
#include "pricer/core/VanillaOption.h"
#include "pricer/engines/MonteCarloEngine.h"
#include "pricer/engines/MonteCarloResult.h"
#include "pricer/models/BlackScholesModel.h"

namespace {

// exp(-rT) * N(d2) with S0 = K = 100, r = 5%, sigma = 20%, T = 1.
constexpr double kAnalyticDigitalCallPrice = 0.5323248154537634;

}  // namespace

TEST(DigitalCallPayoffTest, PaysOneAboveTheStrike) {
    const pricer::DigitalCallPayoff digital{100.0};
    EXPECT_EQ(digital(120.0), 1.0);
}

TEST(DigitalCallPayoffTest, PaysZeroBelowTheStrike) {
    const pricer::DigitalCallPayoff digital{100.0};
    EXPECT_EQ(digital(80.0), 0.0);
}

// "Strictly above": a spot exactly at the strike pays nothing.
TEST(DigitalCallPayoffTest, PaysZeroAtTheStrike) {
    const pricer::DigitalCallPayoff digital{100.0};
    EXPECT_EQ(digital(100.0), 0.0);
}

// Unlike a call, the payoff does not grow with the spot: 1 is the maximum.
TEST(DigitalCallPayoffTest, PayoffIsCappedAtOne) {
    const pricer::DigitalCallPayoff digital{100.0};
    EXPECT_EQ(digital(1'000'000.0), 1.0);
}

TEST(DigitalCallPayoffTest, StrikeAccessorReturnsConstructorValue) {
    const pricer::DigitalCallPayoff digital{95.0};
    EXPECT_EQ(digital.strike(), 95.0);
}

TEST(DigitalCallPayoffTest, ThrowsOnInvalidStrike) {
    EXPECT_THROW(pricer::DigitalCallPayoff{0.0}, std::invalid_argument);
    EXPECT_THROW(pricer::DigitalCallPayoff{-10.0}, std::invalid_argument);
    EXPECT_THROW(pricer::DigitalCallPayoff{std::numeric_limits<double>::quiet_NaN()},
                 std::invalid_argument);
}

TEST(DigitalCallPayoffTest, WorksThroughBaseClassReference) {
    const pricer::DigitalCallPayoff digital{100.0};
    const pricer::Payoff& base = digital;
    EXPECT_EQ(base(120.0), 1.0);
    EXPECT_EQ(base(80.0), 0.0);
}

TEST(DigitalCallPayoffTest, MonteCarloPriceMatchesAnalyticFormula) {
    const pricer::BlackScholesModel model{100.0, 0.05, 0.20};
    const pricer::MonteCarloEngine engine{model, 100'000, 42, pricer::SamplingScheme::Antithetic};
    const pricer::VanillaOption digital{std::make_shared<pricer::DigitalCallPayoff>(100.0), 1.0};
    const pricer::MonteCarloResult result = engine.simulate(digital);
    EXPECT_NEAR(result.price, kAnalyticDigitalCallPrice, 5.0 * result.standardError);
}
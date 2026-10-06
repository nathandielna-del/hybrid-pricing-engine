#include "pricer/core/Payoff.h"

#include <gtest/gtest.h>

#include <limits>
#include <memory>
#include <stdexcept>

namespace {

using pricer::CallPayoff;
using pricer::Payoff;
using pricer::PutPayoff;

constexpr double kStrike = 100.0;

// Takes the base class by reference: the call must dispatch to the derived class.
double evaluate(const Payoff& payoff, double spotAtMaturity) {
    return payoff(spotAtMaturity);
}

TEST(PayoffClassTest, CallInTheMoney) {
    const CallPayoff call{kStrike};
    EXPECT_DOUBLE_EQ(call(120.0), 20.0);
}

TEST(PayoffClassTest, CallOutOfTheMoney) {
    const CallPayoff call{kStrike};
    EXPECT_DOUBLE_EQ(call(80.0), 0.0);
}

TEST(PayoffClassTest, CallAtTheMoney) {
    const CallPayoff call{kStrike};
    EXPECT_DOUBLE_EQ(call(kStrike), 0.0);
}

TEST(PayoffClassTest, PutInTheMoney) {
    const PutPayoff put{kStrike};
    EXPECT_DOUBLE_EQ(put(80.0), 20.0);
}

TEST(PayoffClassTest, PutOutOfTheMoney) {
    const PutPayoff put{kStrike};
    EXPECT_DOUBLE_EQ(put(120.0), 0.0);
}

TEST(PayoffClassTest, StrikeAccessorReturnsConstructorValue) {
    const CallPayoff call{kStrike};
    const PutPayoff put{95.0};
    EXPECT_DOUBLE_EQ(call.strike(), kStrike);
    EXPECT_DOUBLE_EQ(put.strike(), 95.0);
}

TEST(PayoffClassTest, MatchesFreeFunctions) {
    const CallPayoff call{kStrike};
    const PutPayoff put{kStrike};
    for (const double spot : {0.0, 50.0, 99.99, 100.0, 100.01, 150.0}) {
        EXPECT_DOUBLE_EQ(call(spot), pricer::callPayoff(spot, kStrike)) << "spot = " << spot;
        EXPECT_DOUBLE_EQ(put(spot), pricer::putPayoff(spot, kStrike)) << "spot = " << spot;
    }
}

TEST(PayoffClassTest, ThrowsOnZeroStrike) {
    EXPECT_THROW(CallPayoff{0.0}, std::invalid_argument);
    EXPECT_THROW(PutPayoff{0.0}, std::invalid_argument);
}

TEST(PayoffClassTest, ThrowsOnNegativeStrike) {
    EXPECT_THROW(CallPayoff{-10.0}, std::invalid_argument);
    EXPECT_THROW(PutPayoff{-10.0}, std::invalid_argument);
}

TEST(PayoffClassTest, ThrowsOnNaNStrike) {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    EXPECT_THROW(CallPayoff{nan}, std::invalid_argument);
    EXPECT_THROW(PutPayoff{nan}, std::invalid_argument);
}

TEST(PayoffClassTest, PolymorphismThroughBaseReference) {
    const CallPayoff call{kStrike};
    const PutPayoff put{kStrike};
    EXPECT_DOUBLE_EQ(evaluate(call, 120.0), 20.0);
    EXPECT_DOUBLE_EQ(evaluate(put, 80.0), 20.0);
}

TEST(PayoffClassTest, PolymorphismThroughUniquePtr) {
    const std::unique_ptr<Payoff> call = std::make_unique<CallPayoff>(kStrike);
    const std::unique_ptr<Payoff> put = std::make_unique<PutPayoff>(kStrike);
    EXPECT_DOUBLE_EQ((*call)(120.0), 20.0);
    EXPECT_DOUBLE_EQ((*put)(80.0), 20.0);
}

}  // namespace
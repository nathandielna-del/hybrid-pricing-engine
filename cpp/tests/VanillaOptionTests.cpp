#include "pricer/core/VanillaOption.h"

#include <gtest/gtest.h>

#include <limits>
#include <memory>
#include <stdexcept>

namespace {

using pricer::CallPayoff;
using pricer::Payoff;
using pricer::PutPayoff;
using pricer::VanillaOption;

TEST(VanillaOptionTest, StoresMaturity) {
    const VanillaOption option{std::make_shared<CallPayoff>(100.0), 1.5};
    EXPECT_DOUBLE_EQ(option.maturity(), 1.5);
}

TEST(VanillaOptionTest, PayoffAtDelegatesToCallPayoff) {
    const VanillaOption option{std::make_shared<CallPayoff>(100.0), 1.0};
    EXPECT_DOUBLE_EQ(option.payoffAt(120.0), 20.0);
    EXPECT_DOUBLE_EQ(option.payoffAt(80.0), 0.0);
}

TEST(VanillaOptionTest, PayoffAtDelegatesToPutPayoff) {
    const VanillaOption option{std::make_shared<PutPayoff>(100.0), 1.0};
    EXPECT_DOUBLE_EQ(option.payoffAt(80.0), 20.0);
    EXPECT_DOUBLE_EQ(option.payoffAt(120.0), 0.0);
}

TEST(VanillaOptionTest, PayoffAccessorReturnsTheStoredObject) {
    const auto payoff = std::make_shared<CallPayoff>(100.0);
    const VanillaOption option{payoff, 1.0};
    EXPECT_EQ(&option.payoff(), payoff.get());
}

TEST(VanillaOptionTest, ThrowsOnNullPayoff) {
    EXPECT_THROW(VanillaOption(nullptr, 1.0), std::invalid_argument);
}

TEST(VanillaOptionTest, ThrowsOnInvalidMaturity) {
    const auto payoff = std::make_shared<CallPayoff>(100.0);
    EXPECT_THROW(VanillaOption(payoff, 0.0), std::invalid_argument);
    EXPECT_THROW(VanillaOption(payoff, -1.0), std::invalid_argument);
    EXPECT_THROW(VanillaOption(payoff, std::numeric_limits<double>::quiet_NaN()),
                 std::invalid_argument);
}

TEST(VanillaOptionTest, CopiesShareTheSamePayoff) {
    const auto payoff = std::make_shared<CallPayoff>(100.0);
    const VanillaOption original{payoff, 1.0};
    const VanillaOption copy = original;  // Rule of Zero: copy works out of the box

    EXPECT_EQ(&copy.payoff(), &original.payoff());
    EXPECT_EQ(payoff.use_count(), 3);  // local variable + original + copy
}

}  // namespace
#include "pricer/models/BlackScholesModel.h"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

namespace {

using pricer::BlackScholesModel;

constexpr double kTolerance = 1e-12;
constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();
constexpr double kInfinity = std::numeric_limits<double>::infinity();

TEST(BlackScholesModelTest, StoresParameters) {
    // Distinct values: catches any swap between constructor arguments.
    const BlackScholesModel model{100.0, 0.05, 0.2};
    EXPECT_DOUBLE_EQ(model.spot(), 100.0);
    EXPECT_DOUBLE_EQ(model.rate(), 0.05);
    EXPECT_DOUBLE_EQ(model.volatility(), 0.2);
}

TEST(BlackScholesModelTest, AcceptsNegativeRate) {
    const BlackScholesModel model{100.0, -0.01, 0.2};
    EXPECT_DOUBLE_EQ(model.rate(), -0.01);
}

TEST(BlackScholesModelTest, DiscountFactorMatchesReference) {
    const BlackScholesModel model{100.0, 0.05, 0.2};
    EXPECT_NEAR(model.discountFactor(1.0), 0.951229424500714, kTolerance);
}

TEST(BlackScholesModelTest, ForwardMatchesReference) {
    const BlackScholesModel model{100.0, 0.05, 0.2};
    EXPECT_NEAR(model.forward(1.0), 105.12710963760241, kTolerance);
}

TEST(BlackScholesModelTest, ZeroMaturityGivesNoDiscountingAndForwardEqualsSpot) {
    const BlackScholesModel model{100.0, 0.05, 0.2};
    EXPECT_DOUBLE_EQ(model.discountFactor(0.0), 1.0);
    EXPECT_DOUBLE_EQ(model.forward(0.0), 100.0);
}

TEST(BlackScholesModelTest, ForwardTimesDiscountFactorEqualsSpot) {
    // F(T) * DF(T) = S0 * exp(rT) * exp(-rT) = S0, for any maturity.
    const BlackScholesModel model{100.0, 0.05, 0.2};
    for (const double maturity : {0.25, 1.0, 5.0, 30.0}) {
        EXPECT_NEAR(model.forward(maturity) * model.discountFactor(maturity), 100.0, 1e-10)
            << "maturity = " << maturity;
    }
}

TEST(BlackScholesModelTest, ThrowsOnInvalidSpot) {
    EXPECT_THROW(BlackScholesModel(0.0, 0.05, 0.2), std::invalid_argument);
    EXPECT_THROW(BlackScholesModel(-100.0, 0.05, 0.2), std::invalid_argument);
    EXPECT_THROW(BlackScholesModel(kNaN, 0.05, 0.2), std::invalid_argument);
}

TEST(BlackScholesModelTest, ThrowsOnInvalidVolatility) {
    EXPECT_THROW(BlackScholesModel(100.0, 0.05, 0.0), std::invalid_argument);
    EXPECT_THROW(BlackScholesModel(100.0, 0.05, -0.2), std::invalid_argument);
    EXPECT_THROW(BlackScholesModel(100.0, 0.05, kNaN), std::invalid_argument);
}

TEST(BlackScholesModelTest, ThrowsOnNonFiniteRate) {
    EXPECT_THROW(BlackScholesModel(100.0, kNaN, 0.2), std::invalid_argument);
    EXPECT_THROW(BlackScholesModel(100.0, kInfinity, 0.2), std::invalid_argument);
}

TEST(BlackScholesModelTest, ErrorMessageNamesTheParameter) {
    try {
        [[maybe_unused]] const BlackScholesModel model{100.0, 0.05, 0.0};
        FAIL() << "Expected std::invalid_argument";
    } catch (const std::invalid_argument& error) {
        EXPECT_STREQ(error.what(), "BlackScholesModel: volatility must be strictly positive.");
    }
}

TEST(BlackScholesModelTest, TerminalSpotWithZeroDrawIsDriftOnly) {
    const pricer::BlackScholesModel model{100.0, 0.05, 0.20};
    EXPECT_NEAR(model.terminalSpot(1.0, 0.0), 103.0454533953517, 1e-12);
}

TEST(BlackScholesModelTest, TerminalSpotWithPositiveDrawGoesUp) {
    const pricer::BlackScholesModel model{100.0, 0.05, 0.20};
    EXPECT_NEAR(model.terminalSpot(1.0, 1.0), 125.86000099294779, 1e-12);
}

TEST(BlackScholesModelTest, TerminalSpotWithNegativeDrawGoesDown) {
    const pricer::BlackScholesModel model{100.0, 0.05, 0.20};
    EXPECT_NEAR(model.terminalSpot(1.0, -1.0), 84.36648165963837, 1e-12);
}

TEST(BlackScholesModelTest, TerminalSpotScalesWithSquareRootOfMaturity) {
    const pricer::BlackScholesModel model{100.0, 0.05, 0.20};
    EXPECT_NEAR(model.terminalSpot(2.0, 1.5), 162.29801673524628, 1e-12);
}

}  // namespace
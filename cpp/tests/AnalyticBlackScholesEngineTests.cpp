#include "pricer/engines/AnalyticBlackScholesEngine.h"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

namespace {

using pricer::AnalyticBlackScholesEngine;
using pricer::BlackScholesModel;
using pricer::BlackScholesParams;
using pricer::CallPayoff;
using pricer::Greeks;
using pricer::OptionType;
using pricer::Payoff;
using pricer::PricingEngine;
using pricer::PutPayoff;
using pricer::VanillaOption;

constexpr double kTolerance = 1e-10;

// A payoff with no closed-form support in this engine: pays 1 if S_T > K.
class DigitalCallPayoff final : public Payoff {
public:
    explicit DigitalCallPayoff(double strike) : strike_{strike} {}

    [[nodiscard]] double operator()(double spotAtMaturity) const noexcept override {
        return spotAtMaturity > strike_ ? 1.0 : 0.0;
    }

private:
    double strike_;
};

BlackScholesModel referenceModel() {
    return BlackScholesModel{100.0, 0.05, 0.2};
}

TEST(AnalyticBlackScholesEngineTest, CallMatchesReferencePrice) {
    const AnalyticBlackScholesEngine engine{referenceModel()};
    const VanillaOption call{std::make_shared<CallPayoff>(100.0), 1.0};
    EXPECT_NEAR(engine.price(call), 10.450583572185565, kTolerance);
}

TEST(AnalyticBlackScholesEngineTest, PutMatchesReferencePrice) {
    const AnalyticBlackScholesEngine engine{referenceModel()};
    const VanillaOption put{std::make_shared<PutPayoff>(100.0), 1.0};
    EXPECT_NEAR(engine.price(put), 5.573526022256971, kTolerance);
}

TEST(AnalyticBlackScholesEngineTest, MatchesSprint1FreeFunctions) {
    const AnalyticBlackScholesEngine engine{referenceModel()};
    const VanillaOption call{std::make_shared<CallPayoff>(110.0), 0.5};
    const VanillaOption put{std::make_shared<PutPayoff>(110.0), 0.5};
    const BlackScholesParams params{
        .spot = 100.0,
        .strike = 110.0,
        .rate = 0.05,
        .volatility = 0.2,
        .maturity = 0.5,
    };
    EXPECT_DOUBLE_EQ(engine.price(call), pricer::blackScholesPrice(params, OptionType::Call));
    EXPECT_DOUBLE_EQ(engine.price(put), pricer::blackScholesPrice(params, OptionType::Put));
}

TEST(AnalyticBlackScholesEngineTest, SatisfiesPutCallParity) {
    // C - P = S0 - K * exp(-rT)
    const BlackScholesModel model = referenceModel();
    const AnalyticBlackScholesEngine engine{model};
    for (const double strike : {80.0, 100.0, 120.0}) {
        const VanillaOption call{std::make_shared<CallPayoff>(strike), 2.0};
        const VanillaOption put{std::make_shared<PutPayoff>(strike), 2.0};
        const double expected = model.spot() - strike * model.discountFactor(2.0);
        EXPECT_NEAR(engine.price(call) - engine.price(put), expected, kTolerance)
            << "strike = " << strike;
    }
}

TEST(AnalyticBlackScholesEngineTest, WorksThroughBaseClassReference) {
    const AnalyticBlackScholesEngine analytic{referenceModel()};
    const PricingEngine& engine = analytic;
    const VanillaOption call{std::make_shared<CallPayoff>(100.0), 1.0};
    EXPECT_NEAR(engine.price(call), 10.450583572185565, kTolerance);
}

TEST(AnalyticBlackScholesEngineTest, PutGreeksMatchSprint1FreeFunctions) {
    const AnalyticBlackScholesEngine engine{referenceModel()};
    const VanillaOption put{std::make_shared<PutPayoff>(100.0), 1.0};
    const BlackScholesParams params{
        .spot = 100.0,
        .strike = 100.0,
        .rate = 0.05,
        .volatility = 0.2,
        .maturity = 1.0,
    };
    const Greeks actual = engine.greeks(put);
    const Greeks expected = pricer::blackScholesGreeks(params, OptionType::Put);
    EXPECT_DOUBLE_EQ(actual.delta, expected.delta);
    EXPECT_DOUBLE_EQ(actual.gamma, expected.gamma);
    EXPECT_DOUBLE_EQ(actual.vega, expected.vega);
    EXPECT_DOUBLE_EQ(actual.theta, expected.theta);
    EXPECT_DOUBLE_EQ(actual.rho, expected.rho);
    EXPECT_LT(actual.delta, 0.0);  // a put always has a negative delta
}

TEST(AnalyticBlackScholesEngineTest, ThrowsOnUnsupportedPayoff) {
    const AnalyticBlackScholesEngine engine{referenceModel()};
    const VanillaOption digital{std::make_shared<DigitalCallPayoff>(100.0), 1.0};
    EXPECT_THROW((void)engine.price(digital), std::invalid_argument);
    EXPECT_THROW((void)engine.greeks(digital), std::invalid_argument);
}

}  // namespace
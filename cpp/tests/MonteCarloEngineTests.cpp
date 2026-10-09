#include "pricer/engines/MonteCarloEngine.h"

#include <cmath>
#include <memory>
#include <stdexcept>

#include <gtest/gtest.h>

#include "pricer/core/Payoff.h"
#include "pricer/core/VanillaOption.h"
#include "pricer/engines/MonteCarloResult.h"
#include "pricer/engines/PricingEngine.h"
#include "pricer/models/BlackScholesModel.h"

namespace {

// Reference values: S0 = 100, K = 100, r = 5%, sigma = 20%, T = 1.
constexpr double kAnalyticCallPrice = 10.450583572185565;
constexpr double kAnalyticPutPrice = 5.573526022256971;
constexpr double kAnalyticDigitalCallPrice = 0.5323248154537634;  // exp(-rT) * N(d2)

constexpr std::size_t kPaths = 100'000;
constexpr std::uint64_t kSeed = 42;

// With 100,000 paths the standard errors are about 0.047 (call), 0.027 (put) and
// 0.0015 (digital). Each tolerance is about 5 standard errors: a correct engine never
// fails, a wrong formula (missing discounting, wrong drift...) always does.
constexpr double kCallTolerance = 0.25;
constexpr double kPutTolerance = 0.15;
constexpr double kDigitalTolerance = 0.008;

// Pays 1 if the spot ends above the strike, 0 otherwise. No closed form in our analytic
// engine, but Monte-Carlo prices it without any change.
class DigitalCallPayoff final : public pricer::Payoff {
public:
    explicit DigitalCallPayoff(double strike) : strike_{strike} {}

    double operator()(double spotAtMaturity) const override {
        return spotAtMaturity > strike_ ? 1.0 : 0.0;
    }

private:
    double strike_;
};

pricer::BlackScholesModel referenceModel() {
    return pricer::BlackScholesModel{100.0, 0.05, 0.20};
}

pricer::VanillaOption atTheMoneyCall() {
    return pricer::VanillaOption{std::make_shared<pricer::CallPayoff>(100.0), 1.0};
}

pricer::VanillaOption atTheMoneyPut() {
    return pricer::VanillaOption{std::make_shared<pricer::PutPayoff>(100.0), 1.0};
}

}  // namespace

TEST(MonteCarloEngineTest, StoresSettings) {
    const pricer::MonteCarloEngine engine{referenceModel(), kPaths, kSeed};
    EXPECT_EQ(engine.numberOfPaths(), kPaths);
    EXPECT_EQ(engine.seed(), kSeed);
}

TEST(MonteCarloEngineTest, ThrowsOnZeroPaths) {
    EXPECT_THROW((pricer::MonteCarloEngine{referenceModel(), 0, kSeed}), std::invalid_argument);
}

TEST(MonteCarloEngineTest, CallIsCloseToAnalyticPrice) {
    const pricer::MonteCarloEngine engine{referenceModel(), kPaths, kSeed};
    EXPECT_NEAR(engine.price(atTheMoneyCall()), kAnalyticCallPrice, kCallTolerance);
}

TEST(MonteCarloEngineTest, PutIsCloseToAnalyticPrice) {
    const pricer::MonteCarloEngine engine{referenceModel(), kPaths, kSeed};
    EXPECT_NEAR(engine.price(atTheMoneyPut()), kAnalyticPutPrice, kPutTolerance);
}

TEST(MonteCarloEngineTest, PricesDigitalCallWithoutAnyChange) {
    const pricer::MonteCarloEngine engine{referenceModel(), kPaths, kSeed};
    const pricer::VanillaOption digital{std::make_shared<DigitalCallPayoff>(100.0), 1.0};
    EXPECT_NEAR(engine.price(digital), kAnalyticDigitalCallPrice, kDigitalTolerance);
}

TEST(MonteCarloEngineTest, SameEngineCalledTwiceGivesSamePrice) {
    const pricer::MonteCarloEngine engine{referenceModel(), 10'000, kSeed};
    const pricer::VanillaOption call = atTheMoneyCall();
    EXPECT_EQ(engine.price(call), engine.price(call));
}

TEST(MonteCarloEngineTest, SameSeedGivesSamePrice) {
    const pricer::MonteCarloEngine first{referenceModel(), 10'000, kSeed};
    const pricer::MonteCarloEngine second{referenceModel(), 10'000, kSeed};
    EXPECT_EQ(first.price(atTheMoneyCall()), second.price(atTheMoneyCall()));
}

TEST(MonteCarloEngineTest, DifferentSeedsGiveDifferentPrices) {
    const pricer::MonteCarloEngine first{referenceModel(), 10'000, kSeed};
    const pricer::MonteCarloEngine second{referenceModel(), 10'000, kSeed + 1};
    EXPECT_NE(first.price(atTheMoneyCall()), second.price(atTheMoneyCall()));
}

TEST(MonteCarloEngineTest, WorksThroughBaseClassReference) {
    const pricer::MonteCarloEngine engine{referenceModel(), kPaths, kSeed};
    const pricer::PricingEngine& base = engine;
    EXPECT_EQ(base.price(atTheMoneyCall()), engine.price(atTheMoneyCall()));
}

// standard error and confidence interval

namespace {

// Exact standard deviations of the discounted payoff, S0 = K = 100, r = 5%, sigma = 20%, T = 1.
// The standard error with N paths is (standard deviation) / sqrt(N).
constexpr double kCallPayoffStdDev = 14.719404091133134;
constexpr double kDigitalPayoffStdDev = 0.47222168385584423;  // exp(-rT) * sqrt(p * (1 - p))

// Always pays the same amount: the payoffs have zero variance.
class ConstantPayoff final : public pricer::Payoff {
public:
    explicit ConstantPayoff(double amount) : amount_{amount} {}

    double operator()(double /*spotAtMaturity*/) const override {
        return amount_;
    }

private:
    double amount_;
};

}  // namespace

TEST(MonteCarloEngineTest, ThrowsOnOnePath) {
    EXPECT_THROW((pricer::MonteCarloEngine{referenceModel(), 1, kSeed}), std::invalid_argument);
}

TEST(MonteCarloEngineTest, AcceptsTwoPaths) {
    EXPECT_NO_THROW((pricer::MonteCarloEngine{referenceModel(), 2, kSeed}));
}

TEST(MonteCarloEngineTest, SimulateAndPriceGiveTheSamePrice) {
    const pricer::MonteCarloEngine engine{referenceModel(), 10'000, kSeed};
    const pricer::VanillaOption call = atTheMoneyCall();
    EXPECT_EQ(engine.simulate(call).price, engine.price(call));
}

TEST(MonteCarloEngineTest, ConstantPayoffHasExactPriceAndZeroError) {
    const pricer::MonteCarloEngine engine{referenceModel(), 10'000, kSeed};
    const pricer::VanillaOption constant{std::make_shared<ConstantPayoff>(7.0), 1.0};
    const pricer::MonteCarloResult result = engine.simulate(constant);
    EXPECT_NEAR(result.price, 7.0 * 0.951229424500714, 1e-12);  // 7 * exp(-0.05)
    EXPECT_NEAR(result.standardError, 0.0, 1e-12);
}

// The estimated standard error is itself random. Over 300 seeds it stayed within 1.5% (call)
// and 0.2% (digital) of the exact value. The tolerances (about 3% and 1.3%) are wider than
// that, but tight enough to catch a standard error that is not discounted (+5%).
TEST(MonteCarloEngineTest, CallStandardErrorMatchesTheory) {
    const pricer::MonteCarloEngine engine{referenceModel(), kPaths, kSeed};
    const double expected = kCallPayoffStdDev / std::sqrt(static_cast<double>(kPaths));
    EXPECT_NEAR(engine.simulate(atTheMoneyCall()).standardError, expected, 0.0015);
}

TEST(MonteCarloEngineTest, DigitalStandardErrorMatchesTheory) {
    const pricer::MonteCarloEngine engine{referenceModel(), kPaths, kSeed};
    const pricer::VanillaOption digital{std::make_shared<DigitalCallPayoff>(100.0), 1.0};
    const double expected = kDigitalPayoffStdDev / std::sqrt(static_cast<double>(kPaths));
    EXPECT_NEAR(engine.simulate(digital).standardError, expected, 0.00002);
}

// 4 times more paths -> standard error divided by sqrt(4) = 2.
TEST(MonteCarloEngineTest, StandardErrorHalvesWithFourTimesMorePaths) {
    const pricer::MonteCarloEngine small{referenceModel(), 25'000, kSeed};
    const pricer::MonteCarloEngine large{referenceModel(), 100'000, kSeed};
    const double ratio = large.simulate(atTheMoneyCall()).standardError /
                         small.simulate(atTheMoneyCall()).standardError;
    EXPECT_NEAR(ratio, 0.5, 0.05);
}

// A correct engine is within 5 standard errors of the true price (probability > 99.9999%).
TEST(MonteCarloEngineTest, AnalyticPriceIsWithinFiveStandardErrors) {
    const pricer::MonteCarloEngine engine{referenceModel(), kPaths, kSeed};
    const pricer::MonteCarloResult call = engine.simulate(atTheMoneyCall());
    const pricer::MonteCarloResult put = engine.simulate(atTheMoneyPut());
    EXPECT_NEAR(call.price, kAnalyticCallPrice, 5.0 * call.standardError);
    EXPECT_NEAR(put.price, kAnalyticPutPrice, 5.0 * put.standardError);
}
#include "pricer/engines/MonteCarloEngine.h"

#include <memory>
#include <stdexcept>

#include <gtest/gtest.h>

#include "pricer/core/Payoff.h"
#include "pricer/core/VanillaOption.h"
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
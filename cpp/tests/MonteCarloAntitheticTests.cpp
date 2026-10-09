#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>

#include <gtest/gtest.h>

#include "pricer/core/Payoff.h"
#include "pricer/core/VanillaOption.h"
#include "pricer/engines/MonteCarloEngine.h"
#include "pricer/engines/MonteCarloResult.h"
#include "pricer/models/BlackScholesModel.h"

namespace {

// Reference values: S0 = 100, K = 100, r = 5%, sigma = 20%, T = 1.
constexpr double kAnalyticCallPrice = 10.450583572185565;
constexpr double kAnalyticPutPrice = 5.573526022256971;
constexpr double kAnalyticDigitalCallPrice = 0.5323248154537634;

constexpr std::uint64_t kSeed = 42;

// Equal cost comparison: 50,000 antithetic samples use 100,000 paths, like 100,000 standard ones.
constexpr std::size_t kAntitheticSamples = 50'000;
constexpr std::size_t kStandardSamples = 100'000;

// Pays 1 if the spot ends above the strike, 0 otherwise.
class DigitalCallPayoff final : public pricer::Payoff {
public:
    explicit DigitalCallPayoff(double strike) : strike_{strike} {}

    double operator()(double spotAtMaturity) const override {
        return spotAtMaturity > strike_ ? 1.0 : 0.0;
    }

private:
    double strike_;
};

// Pays |S_T - K| (a call plus a put): symmetric, it gains when the spot moves in either direction.
class StraddlePayoff final : public pricer::Payoff {
public:
    explicit StraddlePayoff(double strike) : strike_{strike} {}

    double operator()(double spotAtMaturity) const override {
        return std::abs(spotAtMaturity - strike_);
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

pricer::VanillaOption atTheMoneyDigital() {
    return pricer::VanillaOption{std::make_shared<DigitalCallPayoff>(100.0), 1.0};
}

pricer::VanillaOption atTheMoneyStraddle() {
    return pricer::VanillaOption{std::make_shared<StraddlePayoff>(100.0), 1.0};
}

pricer::MonteCarloEngine antitheticEngine(std::size_t samples, std::uint64_t seed) {
    return pricer::MonteCarloEngine{referenceModel(), samples, seed,
                                    pricer::SamplingScheme::Antithetic};
}

pricer::MonteCarloEngine standardEngine(std::size_t samples, std::uint64_t seed) {
    return pricer::MonteCarloEngine{referenceModel(), samples, seed};
}

// Standard error of the antithetic engine divided by the one of the standard engine, at equal cost.
double equalCostStandardErrorRatio(const pricer::VanillaOption& option, std::uint64_t seed) {
    const pricer::MonteCarloResult antithetic =
        antitheticEngine(kAntitheticSamples, seed).simulate(option);
    const pricer::MonteCarloResult standard =
        standardEngine(kStandardSamples, seed).simulate(option);
    return antithetic.standardError / standard.standardError;
}

}  // namespace

TEST(MonteCarloAntitheticTest, DefaultSchemeIsStandard) {
    const pricer::MonteCarloEngine engine{referenceModel(), 1'000, kSeed};
    EXPECT_EQ(engine.scheme(), pricer::SamplingScheme::Standard);
}

TEST(MonteCarloAntitheticTest, StoresAntitheticScheme) {
    EXPECT_EQ(antitheticEngine(1'000, kSeed).scheme(), pricer::SamplingScheme::Antithetic);
}

TEST(MonteCarloAntitheticTest, AntitheticAndStandardGiveDifferentPrices) {
    const double antithetic = antitheticEngine(10'000, kSeed).price(atTheMoneyCall());
    const double standard = standardEngine(10'000, kSeed).price(atTheMoneyCall());
    EXPECT_NE(antithetic, standard);
}

TEST(MonteCarloAntitheticTest, SameSeedGivesSamePrice) {
    EXPECT_EQ(antitheticEngine(10'000, kSeed).price(atTheMoneyCall()),
              antitheticEngine(10'000, kSeed).price(atTheMoneyCall()));
}

// Antithetic sampling changes the noise, not the expected value: the price is still unbiased.
TEST(MonteCarloAntitheticTest, PricesAreWithinFiveStandardErrorsOfAnalytic) {
    const pricer::MonteCarloEngine engine = antitheticEngine(kAntitheticSamples, kSeed);
    const pricer::MonteCarloResult call = engine.simulate(atTheMoneyCall());
    const pricer::MonteCarloResult put = engine.simulate(atTheMoneyPut());
    const pricer::MonteCarloResult digital = engine.simulate(atTheMoneyDigital());
    EXPECT_NEAR(call.price, kAnalyticCallPrice, 5.0 * call.standardError);
    EXPECT_NEAR(put.price, kAnalyticPutPrice, 5.0 * put.standardError);
    EXPECT_NEAR(digital.price, kAnalyticDigitalCallPrice, 5.0 * digital.standardError);
}

// Expected equal-cost ratios (measured on 4 million draws): call 0.71, put 0.77, digital 0.46.
// Over 200 seeds the ratios stayed far from the bounds used below.
TEST(MonteCarloAntitheticTest, ReducesStandardErrorAtEqualCostForMonotonicPayoffs) {
    EXPECT_LT(equalCostStandardErrorRatio(atTheMoneyCall(), kSeed), 0.80);
    EXPECT_LT(equalCostStandardErrorRatio(atTheMoneyPut(), kSeed), 0.85);
    EXPECT_LT(equalCostStandardErrorRatio(atTheMoneyDigital(), kSeed), 0.55);
}

// A straddle gains when S_T goes up AND when it goes down, so payoff(Z) and payoff(-Z) move
// together (correlation +0.64). Averaging them removes little noise, and at equal cost the
// antithetic engine is worse (expected ratio 1.28).
TEST(MonteCarloAntitheticTest, IncreasesStandardErrorAtEqualCostForStraddle) {
    EXPECT_GT(equalCostStandardErrorRatio(atTheMoneyStraddle(), kSeed), 1.15);
}

// The 95% confidence interval must still be correct with antithetic samples.
TEST(MonteCarloAntitheticTest, ConfidenceIntervalContainsTruePriceNinetyFivePercentOfTheTime) {
    constexpr int kRuns = 1'000;
    int runsContainingTruePrice = 0;
    for (int seed = 0; seed < kRuns; ++seed) {
        const pricer::MonteCarloResult result =
            antitheticEngine(500, static_cast<std::uint64_t>(seed)).simulate(atTheMoneyCall());
        if (result.lowerBound95() <= kAnalyticCallPrice &&
            kAnalyticCallPrice <= result.upperBound95()) {
            ++runsContainingTruePrice;
        }
    }
    const double coverage = static_cast<double>(runsContainingTruePrice) / kRuns;
    EXPECT_NEAR(coverage, 0.95, 0.03);
}
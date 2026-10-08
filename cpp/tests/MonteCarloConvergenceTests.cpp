#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

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
constexpr double kCallPayoffStdDev = 14.719404091133134;  // exact, discounted

// Each path count is 4 times the previous one: the standard error should halve at each step.
const std::vector<std::size_t> kPathCounts{1'000, 4'000, 16'000, 64'000, 256'000};

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

// SE = sigma / sqrt(N), so SE * sqrt(N) should give back sigma for every N.
// Over 500 seeds, SE * sqrt(4000) stayed between 13.8 and 15.7: a 1.5 tolerance is safe.
TEST(MonteCarloConvergenceTest, StandardErrorTimesSqrtOfPathsIsConstant) {
    for (const std::size_t paths : kPathCounts) {
        if (paths < 4'000) {
            continue;  // too few paths: the estimated SE is too noisy for this tolerance
        }
        const pricer::MonteCarloEngine engine{referenceModel(), paths, 42};
        const double scaled =
            engine.simulate(atTheMoneyCall()).standardError * std::sqrt(static_cast<double>(paths));
        EXPECT_NEAR(scaled, kCallPayoffStdDev, 1.5) << "paths = " << paths;
    }
}

// For every N, a correct engine is within 5 standard errors of the analytic price.
TEST(MonteCarloConvergenceTest, PriceIsWithinFiveStandardErrorsForEveryPathCount) {
    for (const std::size_t paths : kPathCounts) {
        const pricer::MonteCarloEngine engine{referenceModel(), paths, 42};
        const pricer::MonteCarloResult call = engine.simulate(atTheMoneyCall());
        const pricer::MonteCarloResult put = engine.simulate(atTheMoneyPut());
        EXPECT_NEAR(call.price, kAnalyticCallPrice, 5.0 * call.standardError)
            << "paths = " << paths;
        EXPECT_NEAR(put.price, kAnalyticPutPrice, 5.0 * put.standardError) << "paths = " << paths;
    }
}

// The real error |MC - analytic| of one run is random. Averaged over 100 seeds, it should be
// divided by sqrt(16) = 4 when the number of paths is multiplied by 16 (expected ratio: 0.25).
// A biased engine (wrong drift, missing discounting) would give a ratio close to 1.
TEST(MonteCarloConvergenceTest, AverageErrorIsDividedByFourWithSixteenTimesMorePaths) {
    constexpr int kSeeds = 100;
    double smallRunsError = 0.0;
    double largeRunsError = 0.0;
    for (int seed = 0; seed < kSeeds; ++seed) {
        const auto smallSeed = static_cast<std::uint64_t>(seed);
        const auto largeSeed = static_cast<std::uint64_t>(seed + 1'000);
        const pricer::MonteCarloEngine small{referenceModel(), 1'000, smallSeed};
        const pricer::MonteCarloEngine large{referenceModel(), 16'000, largeSeed};
        smallRunsError += std::abs(small.price(atTheMoneyCall()) - kAnalyticCallPrice);
        largeRunsError += std::abs(large.price(atTheMoneyCall()) - kAnalyticCallPrice);
    }
    const double ratio = largeRunsError / smallRunsError;
    EXPECT_GT(ratio, 0.12);
    EXPECT_LT(ratio, 0.45);
}

// Definition of a 95% confidence interval: over many independent runs, about 95% of the
// intervals contain the true price. With 1,000 runs, the observed fraction has a standard
// deviation of sqrt(0.95 * 0.05 / 1000) ~ 0.007, so 0.03 is about 4 standard deviations.
TEST(MonteCarloConvergenceTest, ConfidenceIntervalContainsTruePriceNinetyFivePercentOfTheTime) {
    constexpr int kRuns = 1'000;
    int runsContainingTruePrice = 0;
    for (int seed = 0; seed < kRuns; ++seed) {
        const pricer::MonteCarloEngine engine{referenceModel(), 1'000,
                                              static_cast<std::uint64_t>(seed)};
        const pricer::MonteCarloResult result = engine.simulate(atTheMoneyCall());
        if (result.lowerBound95() <= kAnalyticCallPrice &&
            kAnalyticCallPrice <= result.upperBound95()) {
            ++runsContainingTruePrice;
        }
    }
    const double coverage = static_cast<double>(runsContainingTruePrice) / kRuns;
    EXPECT_NEAR(coverage, 0.95, 0.03);
}
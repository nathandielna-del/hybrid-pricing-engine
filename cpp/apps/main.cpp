#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "pricer/core/Payoff.h"
#include "pricer/core/VanillaOption.h"
#include "pricer/engines/AnalyticBlackScholesEngine.h"
#include "pricer/engines/MonteCarloEngine.h"
#include "pricer/engines/MonteCarloResult.h"
#include "pricer/engines/PricingEngine.h"
#include "pricer/models/BlackScholesModel.h"

namespace {

// exp(-rT) * N(d2) for S0 = K = 100, r = 5%, sigma = 20%, T = 1. The analytic engine does not
// support digital options, so the reference value is written here.
constexpr double kDigitalCallReferencePrice = 0.5323248154537634;

// Works with any PricingEngine: the analytic engine and the Monte-Carlo engine both plug in here.
void printPortfolio(const pricer::PricingEngine& engine,
                    const std::vector<pricer::VanillaOption>& options) {
    for (const pricer::VanillaOption& option : options) {
        std::cout << "maturity = " << option.maturity() << "  price = " << engine.price(option)
                  << '\n';
    }
}

// One line: label, Monte-Carlo price, standard error, 95% confidence interval, reference price.
void printMonteCarloResult(const std::string& label, const pricer::MonteCarloResult& result,
                           double reference) {
    std::cout << std::left << std::setw(20) << label << std::right << std::setw(9) << result.price
              << "  +/- " << std::setw(6) << result.standardError << "  [" << result.lowerBound95()
              << ", " << result.upperBound95() << "]  ref " << reference << '\n';
}

// Result of one simulation together with the time it took.
struct TimedSimulation {
    pricer::MonteCarloResult result;
    double milliseconds;
};

TimedSimulation timeSimulation(const pricer::MonteCarloEngine& engine,
                               const pricer::VanillaOption& option) {
    // steady_clock never goes backwards: it is the right clock to measure a duration.
    const auto start = std::chrono::steady_clock::now();
    const pricer::MonteCarloResult result = engine.simulate(option);
    const auto end = std::chrono::steady_clock::now();
    const double milliseconds = std::chrono::duration<double, std::milli>(end - start).count();
    return TimedSimulation{.result = result, .milliseconds = milliseconds};
}

// Timings are only meaningful in a Release build (the Debug build is several times slower).
void printBenchmark(const pricer::BlackScholesModel& model, const pricer::VanillaOption& option) {
    const std::vector<std::size_t> pathCounts{10'000, 100'000, 1'000'000};
    for (const std::size_t paths : pathCounts) {
        const pricer::MonteCarloEngine engine{model, paths, 42};
        const TimedSimulation timed = timeSimulation(engine, option);
        const double millionPathsPerSecond =
            static_cast<double>(paths) / timed.milliseconds / 1'000.0;
        std::cout << std::setw(9) << paths << " paths:  price " << timed.result.price << "  +/- "
                  << timed.result.standardError << "  " << std::setw(9) << timed.milliseconds
                  << " ms  (" << millionPathsPerSecond << " M paths/s)\n";
    }
}

}  // namespace

int main() {
    try {
        std::cout << std::fixed << std::setprecision(4);

        // Market data (model), then the pricing methods built on it (engines).
        const pricer::BlackScholesModel model{100.0, 0.05, 0.2};
        const pricer::AnalyticBlackScholesEngine engine{model};

        // Products: what is paid and when.
        std::vector<pricer::VanillaOption> options;
        options.emplace_back(std::make_shared<pricer::CallPayoff>(100.0), 1.0);
        options.emplace_back(std::make_shared<pricer::PutPayoff>(100.0), 1.0);
        options.emplace_back(std::make_shared<pricer::CallPayoff>(110.0), 0.5);

        std::cout << "--- Portfolio (analytic Black-Scholes) ---\n";
        printPortfolio(engine, options);

        const pricer::Greeks greeks = engine.greeks(options.front());
        std::cout << "--- Greeks of the first option ---\n"
                  << "delta = " << greeks.delta << '\n'
                  << "gamma = " << greeks.gamma << '\n'
                  << "vega  = " << greeks.vega << '\n'
                  << "theta = " << greeks.theta << '\n'
                  << "rho   = " << greeks.rho << '\n';

        // Same portfolio, same function, different engine: this is what PricingEngine is for.
        const pricer::MonteCarloEngine standard{model, 100'000, 42};
        std::cout << "--- Portfolio (Monte-Carlo, 100,000 paths) ---\n";
        printPortfolio(standard, options);

        // Equal cost: 50,000 antithetic samples use 100,000 paths.
        const pricer::MonteCarloEngine antithetic{model, 50'000, 42,
                                                  pricer::SamplingScheme::Antithetic};
        const pricer::VanillaOption& call = options[0];
        const pricer::VanillaOption& put = options[1];
        const pricer::VanillaOption digital{std::make_shared<pricer::DigitalCallPayoff>(100.0),
                                            1.0};

        std::cout << "--- Monte-Carlo vs reference (100,000 paths, seed 42) ---\n";
        printMonteCarloResult("Call standard", standard.simulate(call), engine.price(call));
        printMonteCarloResult("Call antithetic", antithetic.simulate(call), engine.price(call));
        printMonteCarloResult("Put standard", standard.simulate(put), engine.price(put));
        printMonteCarloResult("Put antithetic", antithetic.simulate(put), engine.price(put));
        printMonteCarloResult("Digital standard", standard.simulate(digital),
                              kDigitalCallReferencePrice);
        printMonteCarloResult("Digital antithetic", antithetic.simulate(digital),
                              kDigitalCallReferencePrice);

        std::cout << "--- Benchmark (ATM call, standard sampling) ---\n";
        printBenchmark(model, call);

        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}

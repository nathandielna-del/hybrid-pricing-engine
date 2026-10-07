#include "pricer/engines/AnalyticBlackScholesEngine.h"

#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

namespace {

// Works with any PricingEngine: a Monte-Carlo engine will plug in here in Sprint 3.
void printPortfolio(const pricer::PricingEngine& engine,
                    const std::vector<pricer::VanillaOption>& options) {
    for (const pricer::VanillaOption& option : options) {
        std::cout << "maturity = " << option.maturity() << "  price = " << engine.price(option)
                  << '\n';
    }
}

}  // namespace

int main() {
    try {
        std::cout << std::fixed << std::setprecision(4);

        // Market data (model), then the pricing method built on it (engine).
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

        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
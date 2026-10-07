#include "pricer/engines/AnalyticBlackScholesEngine.h"

#include <stdexcept>

namespace pricer {

namespace {

struct VanillaTerms {
    OptionType type;
    double strike;
};

VanillaTerms extractVanillaTerms(const Payoff& payoff) {
    if (const auto* call = dynamic_cast<const CallPayoff*>(&payoff)) {
        return {OptionType::Call, call->strike()};
    }
    if (const auto* put = dynamic_cast<const PutPayoff*>(&payoff)) {
        return {OptionType::Put, put->strike()};
    }
    throw std::invalid_argument(
        "AnalyticBlackScholesEngine: only call and put payoffs are supported.");
}

BlackScholesParams makeParams(const BlackScholesModel& model, const VanillaOption& option,
                              double strike) {
    return BlackScholesParams{
        .spot = model.spot(),
        .strike = strike,
        .rate = model.rate(),
        .volatility = model.volatility(),
        .maturity = option.maturity(),
    };
}

}  // namespace

AnalyticBlackScholesEngine::AnalyticBlackScholesEngine(const BlackScholesModel& model)
    : model_{model} {}

double AnalyticBlackScholesEngine::price(const VanillaOption& option) const {
    const VanillaTerms terms = extractVanillaTerms(option.payoff());
    return blackScholesPrice(makeParams(model_, option, terms.strike), terms.type);
}

Greeks AnalyticBlackScholesEngine::greeks(const VanillaOption& option) const {
    const VanillaTerms terms = extractVanillaTerms(option.payoff());
    return blackScholesGreeks(makeParams(model_, option, terms.strike), terms.type);
}

}  // namespace pricer
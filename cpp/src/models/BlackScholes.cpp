#include "pricer/models/BlackScholes.h"
#include "pricer/math/NormalDistribution.h"

#include <cmath>
#include <stdexcept>

namespace pricer {
namespace {
void validateParams(const BlackScholesParams& p) {
    if (p.spot <= 0.0) {
        throw std::invalid_argument("Spot must be strictly positive");
    }
    if (p.strike <= 0.0) {
        throw std::invalid_argument("Strike must be strictly positive");
    }
    if (p.volatility <= 0.0) {
        throw std::invalid_argument("Volatility must be strictly positive");
    }
    if (p.maturity <= 0.0) {
        throw std::invalid_argument("Maturity must be strictly positive");
    }
}

struct D1D2 {
    double d1;
    double d2;
};

D1D2 computeD1D2(const BlackScholesParams& p) {
    const double sigmaSqrtT = p.volatility * std::sqrt(p.maturity);
    const double d1 =
        (std::log(p.spot / p.strike) + (p.rate + p.volatility * p.volatility / 2.0) * p.maturity) /
        sigmaSqrtT;
    return D1D2{.d1 = d1, .d2 = d1 - sigmaSqrtT};
}
}  // namespace

double blackScholesPrice(const BlackScholesParams& p, OptionType option) {
    validateParams(p);
    const auto [d1, d2] = computeD1D2(p);
    const double discountFactor = std::exp(-p.rate * p.maturity);
    switch (option) {
        case OptionType::Call:
            return p.spot * normalCdf(d1) - p.strike * discountFactor * normalCdf(d2);
        case OptionType::Put:
            return p.strike * discountFactor * normalCdf(-d2) - p.spot * normalCdf(-d1);
    }
    throw std::invalid_argument("Unknown option type");
}

Greeks blackScholesGreeks(const BlackScholesParams& p, OptionType option) {
    validateParams(p);
    const auto [d1, d2] = computeD1D2(p);
    const double sqrtT = std::sqrt(p.maturity);
    const double discountFactor = std::exp(-p.rate * p.maturity);
    const double pdfD1 = normalPdf(d1);
    const double gamma = pdfD1 / (p.spot * p.volatility * sqrtT);
    const double vega = p.spot * pdfD1 * sqrtT;
    const double thetaVolTerm = -(p.spot * pdfD1 * p.volatility) / (2.0 * sqrtT);
    const double discountedStrike = p.strike * discountFactor;
    switch (option) {
        case OptionType::Call:
            return Greeks{.delta = normalCdf(d1),
                          .gamma = gamma,
                          .vega = vega,
                          .theta = thetaVolTerm - p.rate * discountedStrike * normalCdf(d2),
                          .rho = discountedStrike * p.maturity * normalCdf(d2)};
        case OptionType::Put:
            return Greeks{.delta = normalCdf(d1) - 1.0,
                          .gamma = gamma,
                          .vega = vega,
                          .theta = thetaVolTerm + p.rate * discountedStrike * normalCdf(-d2),
                          .rho = -discountedStrike * p.maturity * normalCdf(-d2)};
    }
    throw std::invalid_argument("Unknown option type");
}

}  // namespace pricer
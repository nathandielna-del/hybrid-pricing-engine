#include "pricer/models/BlackScholes.h"
#include "pricer/math/NormalDistribution.h"

#include <cmath>
#include <stdexcept>

namespace pricer {
    namespace {
        void validateParams(const BlackScholesParams& p) {
            if(p.spot <= 0.0) {
                throw std::invalid_argument("Spot must be strictly positive");
            }
            if(p.strike <= 0.0) {
                throw std::invalid_argument("Strike must be strictly positive");
            }
            if(p.volatility <= 0.0) {
                throw std::invalid_argument("Volatility must be strictly positive");
            }
            if(p.maturity <= 0.0) {
                throw std::invalid_argument("Maturity must be strictly positive");
            }

        }
    } // namespace

    double blackScholesPrice(const BlackScholesParams& p, OptionType option) {
        validateParams(p);
        const double sigmaSqrtT = p.volatility * std::sqrt(p.maturity);
        const double d1 = (std::log(p.spot/p.strike) + (p.rate + p.volatility * p.volatility / 2.0) * p.maturity)/ sigmaSqrtT;
        const double d2 = d1 - sigmaSqrtT;
        const double discountFactor = std::exp(-p.rate * p.maturity);
        switch (option) {
            case OptionType::Call:
                return p.spot * normalCdf(d1) - p.strike * discountFactor * normalCdf(d2);
            case OptionType::Put:
                return p.strike * discountFactor * normalCdf(-d2) - p.spot * normalCdf(-d1);
        }
        throw std::invalid_argument("Unknown option type");
    }

} // namespace pricer
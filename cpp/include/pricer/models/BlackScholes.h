#pragma once

namespace pricer {

enum class OptionType { Call, Put };

struct BlackScholesParams {
    double spot;
    double strike;
    double rate;
    double volatility;
    double maturity;
};

struct Greeks {
    double delta;
    double gamma;
    double vega;
    double theta;
    double rho;
};

[[nodiscard]] double blackScholesPrice(const BlackScholesParams& p, OptionType option);
[[nodiscard]] Greeks blackScholesGreeks(const BlackScholesParams& p, OptionType option);

}  // namespace pricer
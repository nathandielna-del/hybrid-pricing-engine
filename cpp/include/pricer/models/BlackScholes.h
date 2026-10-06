#pragma once

namespace pricer {

    enum class OptionType { Call, Put};

    struct BlackScholesParams {
        double spot;
        double strike;
        double rate;
        double volatility;
        double maturity;
    };

    [[nodiscard]] double blackScholesPrice(const BlackScholesParams& p, OptionType option);
} // namespace pricer
#pragma once

namespace pricer {

// Outcome of a Monte-Carlo pricing: the estimated price and its statistical uncertainty.
struct MonteCarloResult {
    double price;          // discounted average of the simulated payoffs
    double standardError;  // standard deviation of the estimator: sample std dev / sqrt(N)

    // 97.5% quantile of N(0, 1): price +/- 1.96 standard errors is a 95% confidence interval.
    static constexpr double kZ95 = 1.959963984540054;

    [[nodiscard]] double lowerBound95() const noexcept {
        return price - kZ95 * standardError;
    }
    [[nodiscard]] double upperBound95() const noexcept {
        return price + kZ95 * standardError;
    }
};

}  // namespace pricer
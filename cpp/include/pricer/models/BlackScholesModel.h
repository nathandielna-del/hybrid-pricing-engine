#pragma once

namespace pricer {

// Black-Scholes dynamics under the risk-neutral measure:
//   dS_t = r * S_t * dt + sigma * S_t * dW_t
// Holds market data only; the contract itself lives in VanillaOption.
class BlackScholesModel {
public:
    // Throws std::invalid_argument if spot <= 0, volatility <= 0, or rate is not finite.
    BlackScholesModel(double spot, double rate, double volatility);

    [[nodiscard]] double spot() const noexcept;
    [[nodiscard]] double rate() const noexcept;
    [[nodiscard]] double volatility() const noexcept;

    // exp(-r * T). Precondition: maturity >= 0.
    [[nodiscard]] double discountFactor(double maturity) const noexcept;

    // S0 * exp(r * T). Precondition: maturity >= 0.
    [[nodiscard]] double forward(double maturity) const noexcept;

    // Simulates the spot at maturity under the risk-neutral measure (exact GBM solution):
    // S_T = S0 * exp((r - sigma^2 / 2) * T + sigma * sqrt(T) * Z).
    // Precondition: maturity > 0 (already enforced by VanillaOption).
    [[nodiscard]] double terminalSpot(double maturity, double gaussianDraw) const noexcept;

private:
    double spot_;
    double rate_;
    double volatility_;
};

}  // namespace pricer
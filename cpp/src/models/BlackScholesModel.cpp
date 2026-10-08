#include "pricer/models/BlackScholesModel.h"

#include <cmath>
#include <stdexcept>
#include <string>

namespace pricer {

namespace {

double validatedStrictlyPositive(double value, const std::string& name) {
    if (!(value > 0.0)) {
        throw std::invalid_argument("BlackScholesModel: " + name + " must be strictly positive.");
    }
    return value;
}

double validatedFinite(double value, const std::string& name) {
    if (!std::isfinite(value)) {
        throw std::invalid_argument("BlackScholesModel: " + name + " must be finite.");
    }
    return value;
}

}  // namespace

BlackScholesModel::BlackScholesModel(double spot, double rate, double volatility)
    : spot_{validatedStrictlyPositive(spot, "spot")},
      rate_{validatedFinite(rate, "rate")},
      volatility_{validatedStrictlyPositive(volatility, "volatility")} {}

double BlackScholesModel::spot() const noexcept {
    return spot_;
}

double BlackScholesModel::rate() const noexcept {
    return rate_;
}

double BlackScholesModel::volatility() const noexcept {
    return volatility_;
}

double BlackScholesModel::discountFactor(double maturity) const noexcept {
    return std::exp(-rate_ * maturity);
}

double BlackScholesModel::forward(double maturity) const noexcept {
    return spot_ * std::exp(rate_ * maturity);
}

double BlackScholesModel::terminalSpot(double maturity, double gaussianDraw) const noexcept {
    const double drift = (rate_ - 0.5 * volatility_ * volatility_) * maturity;
    const double diffusion = volatility_ * std::sqrt(maturity) * gaussianDraw;
    return spot_ * std::exp(drift + diffusion);
}

}  // namespace pricer
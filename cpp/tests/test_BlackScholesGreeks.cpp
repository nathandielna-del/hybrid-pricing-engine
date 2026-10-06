#include "pricer/models/BlackScholes.h"

#include <gtest/gtest.h>

using pricer::BlackScholesParams;
using pricer::OptionType;

namespace {

const BlackScholesParams atTheMoneyParams{
    .spot = 100.0, .strike = 100.0, .rate = 0.05, .volatility = 0.2, .maturity = 1.0};

const BlackScholesParams inTheMoneyCallParams{
    .spot = 120.0, .strike = 100.0, .rate = 0.03, .volatility = 0.25, .maturity = 2.0};

// Dérivée numérique du prix par rapport à un champ des paramètres :
// (V(x + h) - V(x - h)) / 2h
// `field` désigne le champ à décaler : &BlackScholesParams::spot, ::volatility, etc.
double centralDifference(const BlackScholesParams& params, OptionType type,
                         double BlackScholesParams::*field, double h) {
    BlackScholesParams up = params;
    BlackScholesParams down = params;
    up.*field += h;
    down.*field -= h;
    return (pricer::blackScholesPrice(up, type) - pricer::blackScholesPrice(down, type)) / (2.0 * h);
}

}  // namespace

// --- Valeurs de référence ---

TEST(BlackScholesGreeksTest, CallMatchesReferenceValues) {
    const pricer::Greeks greeks = pricer::blackScholesGreeks(atTheMoneyParams, OptionType::Call);
    EXPECT_NEAR(greeks.delta, 0.6368306511756191, 1e-10);
    EXPECT_NEAR(greeks.gamma, 0.018762017345846895, 1e-10);
    EXPECT_NEAR(greeks.vega, 37.52403469169379, 1e-10);
    EXPECT_NEAR(greeks.theta, -6.414027546438196, 1e-10);
    EXPECT_NEAR(greeks.rho, 53.232481545376345, 1e-10);
}

// --- Identités issues de la parité Call-Put ---

TEST(BlackScholesGreeksTest, SatisfiesPutCallIdentities) {
    for (const BlackScholesParams& params : {atTheMoneyParams, inTheMoneyCallParams}) {
        const pricer::Greeks call = pricer::blackScholesGreeks(params, OptionType::Call);
        const pricer::Greeks put = pricer::blackScholesGreeks(params, OptionType::Put);
        EXPECT_NEAR(call.delta - put.delta, 1.0, 1e-12);
        EXPECT_NEAR(call.gamma, put.gamma, 1e-12);
        EXPECT_NEAR(call.vega, put.vega, 1e-12);
    }
}

// --- Vérification par différences finies ---

TEST(BlackScholesGreeksTest, DeltaMatchesFiniteDifference) {
    for (const BlackScholesParams& params : {atTheMoneyParams, inTheMoneyCallParams}) {
        for (const OptionType type : {OptionType::Call, OptionType::Put}) {
            const double numerical = centralDifference(params, type, &BlackScholesParams::spot, 0.01);
            EXPECT_NEAR(pricer::blackScholesGreeks(params, type).delta, numerical, 1e-4);
        }
    }
}

TEST(BlackScholesGreeksTest, GammaMatchesFiniteDifference) {
    const double h = 0.01;
    for (const BlackScholesParams& params : {atTheMoneyParams, inTheMoneyCallParams}) {
        for (const OptionType type : {OptionType::Call, OptionType::Put}) {
            BlackScholesParams up = params;
            BlackScholesParams down = params;
            up.spot += h;
            down.spot -= h;
            // Dérivée seconde : (V(S + h) - 2V(S) + V(S - h)) / h²
            const double numerical = (pricer::blackScholesPrice(up, type) - 2.0 * pricer::blackScholesPrice(params, type) +
                                      pricer::blackScholesPrice(down, type)) / (h * h);
            EXPECT_NEAR(pricer::blackScholesGreeks(params, type).gamma, numerical, 1e-6);
        }
    }
}

TEST(BlackScholesGreeksTest, VegaMatchesFiniteDifference) {
    for (const BlackScholesParams& params : {atTheMoneyParams, inTheMoneyCallParams}) {
        for (const OptionType type : {OptionType::Call, OptionType::Put}) {
            const double numerical = centralDifference(params, type, &BlackScholesParams::volatility, 1e-4);
            EXPECT_NEAR(pricer::blackScholesGreeks(params, type).vega, numerical, 1e-4);
        }
    }
}

TEST(BlackScholesGreeksTest, RhoMatchesFiniteDifference) {
    for (const BlackScholesParams& params : {atTheMoneyParams, inTheMoneyCallParams}) {
        for (const OptionType type : {OptionType::Call, OptionType::Put}) {
            const double numerical = centralDifference(params, type, &BlackScholesParams::rate, 1e-4);
            EXPECT_NEAR(pricer::blackScholesGreeks(params, type).rho, numerical, 1e-4);
        }
    }
}

TEST(BlackScholesGreeksTest, ThetaMatchesFiniteDifference) {
    for (const BlackScholesParams& params : {atTheMoneyParams, inTheMoneyCallParams}) {
        for (const OptionType type : {OptionType::Call, OptionType::Put}) {
            // Theta = dV/dt = -dV/dT : quand le temps avance, la maturité restante diminue
            const double numerical = -centralDifference(params, type, &BlackScholesParams::maturity, 1e-4);
            EXPECT_NEAR(pricer::blackScholesGreeks(params, type).theta, numerical, 1e-4);
        }
    }
}
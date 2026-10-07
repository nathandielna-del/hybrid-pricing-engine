#include "pricer/models/BlackScholes.h"

#include <gtest/gtest.h>

#include <cmath>
#include <stdexcept>

// Option à la monnaie : spot = strike
const pricer::BlackScholesParams atTheMoneyParams{
    .spot = 100.0, .strike = 100.0, .rate = 0.05, .volatility = 0.2, .maturity = 1.0};

// --- Valeurs de référence ---

TEST(BlackScholesPriceTest, CallMatchesReferenceValue) {
    EXPECT_NEAR(pricer::blackScholesPrice(atTheMoneyParams, pricer::OptionType::Call),
                10.450583572185565, 1e-10);
}

TEST(BlackScholesPriceTest, PutMatchesReferenceValue) {
    EXPECT_NEAR(pricer::blackScholesPrice(atTheMoneyParams, pricer::OptionType::Put),
                5.573526022256971, 1e-10);
}

// --- Parité Call-Put : C - P = S - K * exp(-rT) ---

TEST(BlackScholesPriceTest, SatisfiesPutCallParity) {
    for (const pricer::BlackScholesParams& params : {
             atTheMoneyParams,
             // Call dans la monnaie
             pricer::BlackScholesParams{
                 .spot = 120.0, .strike = 100.0, .rate = 0.03, .volatility = 0.25, .maturity = 2.0},
             // Call hors de la monnaie
             pricer::BlackScholesParams{
                 .spot = 80.0, .strike = 100.0, .rate = 0.02, .volatility = 0.3, .maturity = 0.5},
             // Taux négatif
             pricer::BlackScholesParams{.spot = 100.0,
                                        .strike = 105.0,
                                        .rate = -0.005,
                                        .volatility = 0.15,
                                        .maturity = 1.0},
             // Maturité courte (1 mois)
             pricer::BlackScholesParams{.spot = 100.0,
                                        .strike = 95.0,
                                        .rate = 0.04,
                                        .volatility = 0.4,
                                        .maturity = 1.0 / 12.0},
         }) {
        const double call = pricer::blackScholesPrice(params, pricer::OptionType::Call);
        const double put = pricer::blackScholesPrice(params, pricer::OptionType::Put);
        const double expected =
            params.spot - params.strike * std::exp(-params.rate * params.maturity);
        EXPECT_NEAR(call - put, expected, 1e-10);
    }
}

// --- Entrées invalides ---

TEST(BlackScholesPriceTest, ThrowsOnNegativeSpot) {
    pricer::BlackScholesParams invalid = atTheMoneyParams;
    invalid.spot = -100.0;
    EXPECT_THROW((void)pricer::blackScholesPrice(invalid, pricer::OptionType::Call),
                 std::invalid_argument);
}

TEST(BlackScholesPriceTest, ThrowsOnZeroStrike) {
    pricer::BlackScholesParams invalid = atTheMoneyParams;
    invalid.strike = 0.0;
    EXPECT_THROW((void)pricer::blackScholesPrice(invalid, pricer::OptionType::Call),
                 std::invalid_argument);
}

TEST(BlackScholesPriceTest, ThrowsOnZeroVolatility) {
    pricer::BlackScholesParams invalid = atTheMoneyParams;
    invalid.volatility = 0.0;
    EXPECT_THROW((void)pricer::blackScholesPrice(invalid, pricer::OptionType::Call),
                 std::invalid_argument);
}

TEST(BlackScholesPriceTest, ThrowsOnNegativeMaturity) {
    pricer::BlackScholesParams invalid = atTheMoneyParams;
    invalid.maturity = -1.0;
    EXPECT_THROW((void)pricer::blackScholesPrice(invalid, pricer::OptionType::Put),
                 std::invalid_argument);
}
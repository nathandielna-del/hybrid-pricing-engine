#include "pricer/core/Payoff.h"

#include <gtest/gtest.h>

TEST(PayoffTest, CallInTheMoney) {
    EXPECT_DOUBLE_EQ(pricer::callPayoff(110.0, 100.0), 10.0);
}

TEST(PayoffTest, CallOutOfTheMoney) {
    EXPECT_DOUBLE_EQ(pricer::callPayoff(90.0, 100.0), 0.0);
}

TEST(PayoffTest, PutInTheMoney) {
    EXPECT_DOUBLE_EQ(pricer::putPayoff(90.0, 100.0), 10.0);
}

TEST(PayoffTest, PutOutOfTheMoney) {
    EXPECT_DOUBLE_EQ(pricer::putPayoff(110.0, 100.0), 0.0);
}

// Call - Put = Spot - Strike, quel que soit le spot
TEST(PayoffTest, CallMinusPutEqualsSpotMinusStrike) {
    const double strike = 100.0;
    for (const double spot : {50.0, 90.0, 100.0, 110.0, 150.0}) {
        EXPECT_DOUBLE_EQ(pricer::callPayoff(spot, strike) - pricer::putPayoff(spot, strike), spot - strike);
    }
}
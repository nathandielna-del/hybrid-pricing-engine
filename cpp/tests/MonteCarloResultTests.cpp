#include "pricer/engines/MonteCarloResult.h"

#include <gtest/gtest.h>

TEST(MonteCarloResultTest, BoundsAreCenteredOnThePrice) {
    const pricer::MonteCarloResult result{.price = 10.0, .standardError = 0.5};
    EXPECT_NEAR(result.lowerBound95(), 9.020018007729973, 1e-12);
    EXPECT_NEAR(result.upperBound95(), 10.979981992270027, 1e-12);
}

TEST(MonteCarloResultTest, ZeroStandardErrorGivesAPointInterval) {
    const pricer::MonteCarloResult result{.price = 7.0, .standardError = 0.0};
    EXPECT_EQ(result.lowerBound95(), 7.0);
    EXPECT_EQ(result.upperBound95(), 7.0);
}
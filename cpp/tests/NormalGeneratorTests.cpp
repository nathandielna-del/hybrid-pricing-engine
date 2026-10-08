#include "pricer/math/NormalGenerator.h"

#include <cmath>

#include <gtest/gtest.h>

namespace {

constexpr int kSampleSize = 100'000;

}  // namespace

TEST(NormalGeneratorTest, SameSeedGivesSameSequence) {
    pricer::NormalGenerator first{42};
    pricer::NormalGenerator second{42};
    for (int i = 0; i < 1'000; ++i) {
        EXPECT_EQ(first.next(), second.next()) << "draw index " << i;
    }
}

TEST(NormalGeneratorTest, DifferentSeedsGiveDifferentSequences) {
    pricer::NormalGenerator first{42};
    pricer::NormalGenerator second{43};
    int identicalDraws = 0;
    for (int i = 0; i < 1'000; ++i) {
        if (first.next() == second.next()) {
            ++identicalDraws;
        }
    }
    EXPECT_EQ(identicalDraws, 0);
}

TEST(NormalGeneratorTest, ConsecutiveDrawsDiffer) {
    pricer::NormalGenerator generator{42};
    const double firstDraw = generator.next();
    const double secondDraw = generator.next();
    EXPECT_NE(firstDraw, secondDraw);
}

// Standard error of the sample mean: 1 / sqrt(100000) ~ 0.0032, so 0.02 is ~6 standard errors.
TEST(NormalGeneratorTest, SampleMeanIsCloseToZero) {
    pricer::NormalGenerator generator{42};
    double sum = 0.0;
    for (int i = 0; i < kSampleSize; ++i) {
        sum += generator.next();
    }
    EXPECT_NEAR(sum / kSampleSize, 0.0, 0.02);
}

// Standard error of the sample variance: sqrt(2 / 100000) ~ 0.0045, so 0.03 is ~6 standard errors.
TEST(NormalGeneratorTest, SampleVarianceIsCloseToOne) {
    pricer::NormalGenerator generator{42};
    double sum = 0.0;
    double sumOfSquares = 0.0;
    for (int i = 0; i < kSampleSize; ++i) {
        const double draw = generator.next();
        sum += draw;
        sumOfSquares += draw * draw;
    }
    const double mean = sum / kSampleSize;
    const double variance = sumOfSquares / kSampleSize - mean * mean;
    EXPECT_NEAR(variance, 1.0, 0.03);
}
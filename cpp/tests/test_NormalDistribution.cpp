#include "pricer/math/NormalDistribution.h"

#include <gtest/gtest.h>

TEST(NormalDistributionTest, CdfAtZeroIsOneHalf) {
    EXPECT_DOUBLE_EQ(pricer::normalCdf(0.0), 0.5);
}

TEST(NormalDistributionTest, PdfMatchesReferenceValues) {
    EXPECT_NEAR(pricer::normalPdf(0.0), 0.3989422804014327, 1e-12);
    EXPECT_NEAR(pricer::normalPdf(1.0), 0.24197072451914337, 1e-12);  // aurait détecté l'erreur d'exposant
}

TEST(NormalDistributionTest, CdfMatchesReferenceValues) {
    EXPECT_NEAR(pricer::normalCdf(1.0), 0.8413447460685429, 1e-12);
    EXPECT_NEAR(pricer::normalCdf(1.96), 0.9750021048517795, 1e-12);
}

// N(x) + N(-x) = 1, par symétrie de la gaussienne
TEST(NormalDistributionTest, CdfIsSymmetric) {
    for (const double x : {0.1, 0.5, 1.0, 2.0, 3.5}) {
        EXPECT_NEAR(pricer::normalCdf(x) + pricer::normalCdf(-x), 1.0, 1e-15);
    }
}

// Avec erf au lieu de erfc, ce test échouerait : le résultat serait 0
TEST(NormalDistributionTest, CdfDeepLeftTailIsAccurate) {
    EXPECT_NEAR(pricer::normalCdf(-10.0), 7.619853024160527e-24, 1e-35);
}
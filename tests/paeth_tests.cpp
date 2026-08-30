#include "paeth.hpp"

#include <gtest/gtest.h>

TEST (PaethTest, CalculatePaethPredictor) {
    EXPECT_EQ(paeth_predictor(0, 0, 0), 0);
    EXPECT_EQ(paeth_predictor(1, 1, 1), 1);
    EXPECT_EQ(paeth_predictor(2, 2, 2), 2);
    EXPECT_EQ(paeth_predictor(9, 9, 9), 9);

    EXPECT_EQ(paeth_predictor(10, 20, 30), 10);
    EXPECT_EQ(paeth_predictor(20, 10, 30), 10);
    EXPECT_EQ(paeth_predictor(30, 20, 10), 30);

    EXPECT_EQ(paeth_predictor(255, 0, 0), 255);
    EXPECT_EQ(paeth_predictor(0, 255, 0), 255);
    EXPECT_EQ(paeth_predictor(0, 0, 255), 0);
}
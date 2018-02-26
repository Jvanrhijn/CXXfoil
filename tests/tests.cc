#include <iostream>
#include "../headers/cxxfoil.h"
#include "gtest/gtest.h"

cxxfoil::Xfoil xfoil("/bin/xfoil");

TEST(XfoilTest, Naca) {
  xfoil.NACA("0015");
}

TEST(XfoilTest, SingleAlpha) {
  std::vector<double> result = {5.00, 0.6174, 0.0, -0.00141, -0.0094};
  EXPECT_EQ(result, xfoil.AngleOfAttack(5.0));
  result = {0, 0, 0, -0.00138, 0.0};
  EXPECT_EQ(result, xfoil.AngleOfAttack(0.0));
  result = {-5.00, -0.6174, 0, -0.00141, 0.0094};
  EXPECT_EQ(result, xfoil.AngleOfAttack(-5.0));
}

TEST(XfoilTest, SingleCL) {
  std::vector<double> result = {8.115, 1, 0, -0.00145, -0.0151};
  EXPECT_EQ(result, xfoil.LiftCoefficient(1.0));
  result = {0, 0, 0, -0.00138, 0};
  EXPECT_EQ(result, xfoil.LiftCoefficient(0.0));
  result = {-8.115, -1, 0, -0.00145, 0.0151};
  EXPECT_EQ(result, xfoil.LiftCoefficient(-1.0));
}

TEST(XfoilTest, CLInc) {
  double clstart = 0; double clend = 0.6; double clinc = 0.3;
  size_t len = 1 + (size_t) ceil((clend - clstart) / clinc);
  cxxfoil::polar exp_result(len);
  exp_result.contents[0] = {0, 0, 0, -0.00138, 0};
  exp_result.contents[1] = {2.427, 0.3, 0, -0.00139, -0.0046};
  exp_result.contents[2] = {4.859, 0.6, 0, -0.00141, -0.0091};
  cxxfoil::polar result = xfoil.LiftCoefficient(clstart, clend, clinc);
  for (size_t i = 0; i<len; i++) {
    EXPECT_EQ(exp_result.contents[i], result.contents[i]);
  }
}

TEST(XfoilTest, AlphaInc) {
  double anglest = 0; double anglee = 6; double angleinc = 3;
  size_t len = 1 + (size_t) ceil((anglee - anglest) / angleinc);
  cxxfoil::polar exp_result(len);
  exp_result.contents[0] = {0, 0, 0, -0.00138, 0};
  exp_result.contents[1] = {3, 0.3707, 0, -0.00139, -0.0056};
  exp_result.contents[2] = {6, 0.7405, 0, -0.00142, -0.0112};
  cxxfoil::polar result = xfoil.AngleOfAttack(anglest, anglee, angleinc);
  for (size_t i = 0; i<len; i++) {
    EXPECT_EQ(exp_result.contents[i], result.contents[i]);
  }
}

TEST(XfoilTest, Iter) {
  ASSERT_TRUE(xfoil.SetIterations(200));
}

TEST(XfoilTest, Viscosity) {
  ASSERT_TRUE(xfoil.SetViscosity(150000));
}

TEST(XfoilTest, singleAlphaVisc) {
  std::vector<double> result = {5, 0.688, 0.01586, 0.00505, -0.0207};
  EXPECT_EQ(result, xfoil.AngleOfAttack(5.0));
  result = {0, 0, 0.01327, 0.00563, 0};
  EXPECT_EQ(result, xfoil.AngleOfAttack(0.0));
  result = {-5, -0.6882, 0.01586, 0.00505, 0.0207};
  EXPECT_EQ(result, xfoil.AngleOfAttack(-5.0));
}

TEST(XfoilTest, SingleCLVisc) {
  std::vector<double> result = {9.889, 1, 0.02698, 0.01046, 0.0169};
  EXPECT_EQ(result, xfoil.LiftCoefficient(1.0));
  result = {0, 0, 0.01327, 0.00563, 0};
  EXPECT_EQ(result, xfoil.LiftCoefficient(0.0));
  result = {-9.891, -1, 0.02699, 0.01045, -0.0169};
  EXPECT_EQ(result, xfoil.LiftCoefficient(-1.0));
}

TEST(XfoilTest, CLIncVisc) {
  double clstart = 0; double clend = 0.6; double clinc = 0.3;
  size_t len = 1 + (size_t) ceil((clend - clstart) / clinc);
  cxxfoil::polar exp_result(len);
  exp_result.contents[0] = {0, 0, 0.01327, 0.00563, 0};
  exp_result.contents[1] = {2.141, 0.3, 0.01438, 0.00588, -0.0047};
  exp_result.contents[2] = {3.912, 0.6, 0.01520, 0.00530, -0.0253};
  cxxfoil::polar result = xfoil.LiftCoefficient(clstart, clend, clinc);
  for(size_t i=0; i<len; i++) {
    EXPECT_EQ(exp_result.contents[i], result.contents[i]);
  }
}

TEST(XfoilTest, ConvergenceCheck) {
  xfoil.SetIterations(20);
  EXPECT_ANY_THROW(xfoil.AngleOfAttack(100));
  EXPECT_ANY_THROW(xfoil.LiftCoefficient(100));
  EXPECT_ANY_THROW(xfoil.AngleOfAttack(0, 200, 100));
  EXPECT_ANY_THROW(xfoil.LiftCoefficient(0, 200, 100));
}

TEST(XfoilTest, SecondInstance) {
  cxxfoil::Xfoil xfoil_second("/bin/xfoil");
  xfoil_second.NACA("0015");
  std::vector<double> result = {8.115, 1, 0, -0.00145, -0.0151};
  EXPECT_EQ(result, xfoil_second.LiftCoefficient(1.0));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

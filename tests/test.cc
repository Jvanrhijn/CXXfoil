#include "gtest/gtest.h"
#include "../src/xfoil_config.h"

using namespace cxxfoil;

constexpr char* kPolarKeys[] = {"alpha", "CL", "CD", "CDp", "CM", "Top_Xtr", "Bot_Xtr"};
constexpr double eps = 1e-2;

void checkPolarEqual(const polar& p, const std::vector<double> res) {
 for (int i=0; i<res.size(); i++) {
   auto value = p.at(kPolarKeys[i]);
   ASSERT_NEAR(value[0], res[i], eps);
 }
}

TEST(ConfigBuild, Config) {
  XfoilConfig config("/usr/local/bin/xfoil");
  config.AngleOfAttack(1.0);
  config.LiftCoefficient(1.0);
}

TEST(XfoilRun, AngleInertialSuccess) {
  XfoilConfig config("/usr/local/bin/xfoil");
  config.Naca("2414");
  config.AngleOfAttack(4.0);
  config.PaccRandom();
  auto result = config.GetRunner()
    .Dispatch();

  std::vector<double> expect_results = {4.0, 0.7492, 0.0, -0.00131, -0.0633, 0.0, 0.0};

  checkPolarEqual(result, expect_results);
}

TEST(XfoilRun, LiftCoefficientInertialSuccess) {
  XfoilConfig config("/usr/local/bin/xfoil");
  config.Naca("2414");
  config.LiftCoefficient(1.0);
  config.PaccRandom();
  auto result = config.GetRunner().Dispatch();

  std::vector<double> expect_results = {6.059, 1.0000, 0.00000, -0.00133, -0.0671, 0.0000, 0.0000};

	checkPolarEqual(result, expect_results);

}


int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

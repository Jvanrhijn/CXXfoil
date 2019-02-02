#include "gtest/gtest.h"
#include "../src/xfoil_config.h"

using namespace cxxfoil;

constexpr char* kPolarKeys[] = {"alpha", "CL", "CD", "CDp", "CM", "Top_Xtr", "Bot_Xtr"};

TEST(ConfigBuild, Config) {
  auto config = XfoilConfig("/usr/local/bin/xfoil")
    .AngleOfAttack(1.0)
    .LiftCoefficient(1.0);
}

TEST(XfoilRun, AngleInertialSuccess) {
  auto result = XfoilConfig("/usr/local/bin/xfoil")
    .Naca("2414")
    .AngleOfAttack(4.0)
    .PaccRandom()
    .GetRunner()
    .Dispatch();
  std::vector<double> expect_results = {4.0, 0.7492, 0.0, -0.00131, -0.0633, 0.0, 0.0};

  for (int i=0; i<expect_results.size(); i++) {
    auto value = result[kPolarKeys[i]];
    ASSERT_DOUBLE_EQ(value[0], expect_results[i]);
  }
}


int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

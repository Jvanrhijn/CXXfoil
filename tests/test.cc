#include "gtest/gtest.h"
#include "../src/xfoil_config.h"

using namespace cxxfoil;

const std::vector<std::string> kPolarKeys = {
	"alpha", 
	"CL", 
	"CD", 
	"CDp", 
	"CM", 
	"Top_Xtr", 
	"Bot_Xtr"
};
constexpr double eps = 1e-2;

void checkPolarEqual(const polar& p, const std::vector<double> res) {
 for (size_t i=0; i<res.size(); i++) {
   auto value = p.at(kPolarKeys[i]);
   ASSERT_NEAR(value[0], res[i], eps);
 }
}

TEST(ConfigBuild, NoFoil) {
  XfoilConfig config("/usr/local/bin/xfoil");
	ASSERT_ANY_THROW(config.GetRunner());
}

TEST(XfoilRun, AngleInertialSuccess) {
  XfoilConfig config("/usr/local/bin/xfoil");
  auto result = XfoilConfig("/usr/local/bin/xfoil")
   .Naca("2414")
   .AngleOfAttack(4.0)
   .PaccRandom()
   .GetRunner()
   .Dispatch();

  std::vector<double> expect_results = {4.0, 0.7492, 0.0, -0.00131, -0.0633, 0.0, 0.0};

  checkPolarEqual(result, expect_results);
}

TEST(XfoilRun, LiftCoefficientInertialSuccess) {
  auto result = XfoilConfig("/usr/local/bin/xfoil")
    .Naca("2414")
    .LiftCoefficient(1.0)
    .PaccRandom()
    .GetRunner()
    .Dispatch();

  std::vector<double> expect_results = {6.059, 1.0000, 0.00000, -0.00133, -0.0671, 0.0000, 0.0000};

	checkPolarEqual(result, expect_results);

}

TEST(XfoilRun, AngleViscousSuccess) {
  auto result = XfoilConfig("/usr/local/bin/xfoil")
    .Naca("2414")
    .AngleOfAttack(4.0)
    .Reynolds(100000)
    .PaccRandom()
    .GetRunner()
    .Dispatch();

  std::vector<double> expect_results = {4.000, 0.7278, 0.01780, 0.00982, -0.0614, 0.6233, 1.0000};
	
	checkPolarEqual(result, expect_results);
}

TEST(XfoilRun, LiftCoefficientViscousSuccess) {
  auto result = XfoilConfig("/usr/local/bin/xfoil")
    .Naca("2414")
	  .LiftCoefficient(1.0)
    .Reynolds(100000)
    .PaccRandom()
    .GetRunner()
    .Dispatch();

  std::vector<double> expect_results = {7.121, 1.0000, 0.02106, 0.01277, -0.0443, 0.4234, 1.0000};
	
	checkPolarEqual(result, expect_results);
}

TEST(XfoilRun, AirfoilFile) {
	auto result = XfoilConfig("/usr/local/bin/xfoil")
	  .AirfoilPolarFile("../tests/clarky.dat")
	  .AngleOfAttack(4.0)
	  .PaccRandom()
	  .GetRunner()
    .Dispatch();
	std::vector<double> expect_results = {4.000, 0.8965, 0.00000, -0.00118, -0.0942, 0.0000, 0.0000};
	
	checkPolarEqual(result, expect_results);
}


TEST(XfoilRun, ConvergenceError) {
  auto runner = XfoilConfig("/usr/local/bin/xfoil")
    .Naca("2414")
    .Reynolds(1)
    .GetRunner();
  
  ASSERT_ANY_THROW(runner.Dispatch());
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

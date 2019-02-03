#include <vector>
#include <string>

#include "optional.h"
#include "xfoil_runner.h"

namespace cxxfoil {

enum class Mode {
  ANGLE,
  CL
};

class XfoilConfig {
  public:
    XfoilConfig(std::string binpath) noexcept;

    void AngleOfAttack(double aoa) noexcept;

    void LiftCoefficient(double cl) noexcept;

    void PaccFromString(const std::string& path) noexcept;

    void Naca(const std::string& naca) noexcept;

    void PaccRandom() noexcept;

    XfoilRunner GetRunner();

  private:
    std::string binpath_;
    Mode mode_;

    Optional<double> cl_;
    Optional<double> aoa_;

    Optional<size_t> reynolds_;
    
    Optional<std::string> polar_;
    Optional<std::string> dat_file_;
    
    Optional<std::string> naca_;

};

} // namespace cxxfoil

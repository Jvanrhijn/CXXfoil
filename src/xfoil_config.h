#include <vector>
#include <string>

#include "optional.h"

namespace cxxfoil {

enum class Mode {
  ANGLE,
  CL
};

class XfoilConfig {
  public:
    XfoilConfig(std::string binpath) noexcept;

    XfoilConfig AngleOfAttack(double aoa) noexcept;

    XfoilConfig LiftCoefficient(double cl) noexcept;

    XfoilConfig PaccFromString(std::string path) noexcept;

  private:
    std::string binpath_;
    std::vector<std::string> command_sequence_;
    Mode mode_;

    Optional<double> cl_;
    Optional<double> aoa_;

    Optional<size_t> reynolds_;
    
    Optional<std::string> polar_;
    Optional<std::string> dat_file_;
    
    Optional<std::string> naca_;

};

} // namespace cxxfoil

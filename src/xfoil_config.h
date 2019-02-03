#ifndef _CXXFOIL_SRC_XFOIL_CONFIG_H
#define _CXXFOIL_SRC_XFOIL_CONFIG_H

#include <vector>
#include <string>

#include "optional.h"
#include "xfoil_runner.h"

namespace cxxfoil {

enum class Mode {
  ANGLE,
  CL
};

/**
 * @brief Object representing Xfoil configuration states. The configuration is
 * checked for validity upon creating an XfoilRunner instance.
 */
class XfoilConfig {
  public:
    /** 
     * @param binpath Path to Xfoil binary.
     */
    explicit XfoilConfig(std::string binpath) noexcept;

    /** You can perform either an angle of attack based computation, or a lift
     * coefficient based one. Setting one will overwrite the other.
     * @param aoa Angle of attack to perform computation at.
     */
    void AngleOfAttack(double aoa) noexcept;
  
    /** You can perform either an angle of attack based computation, or a lift
     * coefficient based one. Setting one will overwrite the other.
     * @param cl Lift coefficient to perform computation at.
     */
    void LiftCoefficient(double cl) noexcept;

    /**
     * @brief Create a polar accumulation file at a given path.
     */
    void PaccFromString(const std::string& path) noexcept;

    /**
     * @brief Use a NACA code to set the airfoil.
     */
    void Naca(const std::string& naca) noexcept;

    /**
     * @brief Use the airfoil polar file located at the given path.
     */
    void AirfoilPolarFile(const std::string& datfile) noexcept;

    /**
     * @brief Generate a random polar accumulation filename under /tmp.
     */
    void PaccRandom() noexcept;

    /**
     * @brief Set the Reynolds number for a viscous calculation
     */
    void Reynolds(size_t reynolds) noexcept;

    /**
     * @brief Construct an XfoilRunner instance.
     */
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

#endif // _CXXFOIL_SRC_XFOIL_CONFIG_H

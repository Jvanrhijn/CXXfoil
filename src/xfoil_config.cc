#include "xfoil_config.h"

namespace cxxfoil {

XfoilConfig::XfoilConfig(std::string binpath) noexcept
  : binpath_(std::move(binpath)), mode_(Mode::ANGLE), aoa_(0.0)
{}

XfoilConfig XfoilConfig::AngleOfAttack(double aoa) noexcept {
  mode_ = Mode::ANGLE;
  aoa_ = aoa;
  cl_ = Optional<double>();
  return *this;
}

XfoilConfig XfoilConfig::LiftCoefficient(double cl) noexcept {
  mode_ = Mode::CL;
  cl_ = cl;
  aoa_ = Optional<double>();
  return *this;
}

XfoilConfig XfoilConfig::PaccFromString(std::string path) noexcept {
  polar_ = std::move(path);
  return *this;
}

} // namespace cxxfoil

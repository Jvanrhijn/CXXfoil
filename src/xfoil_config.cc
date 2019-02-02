#include "xfoil_config.h"
#include <cstdio>

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

XfoilConfig XfoilConfig::PaccFromString(const std::string& path) noexcept {
  polar_ = std::move(path);
  return *this;
}

XfoilConfig XfoilConfig::Naca(const std::string& naca) noexcept {
  naca_ = naca;
  return *this;
}

XfoilConfig XfoilConfig::PaccRandom() noexcept {
  std::string polar = std::tmpnam(nullptr);
  polar_ = polar;
  return *this;
}

XfoilRunner XfoilConfig::GetRunner() {
  std::vector<std::string> command_sequence{"plop", "G", "\n"}; 
  if (naca_.has_value()) {
    command_sequence.push_back(naca_.value());
  } else if (dat_file_.has_value()) {
    command_sequence.push_back(dat_file_.value());
  } else {
    throw std::runtime_error("Xfoil cannot run without an airfoil");
  }

  if (reynolds_.has_value()) {
    command_sequence.push_back("oper");
    command_sequence.push_back(std::string("v ") + std::to_string(reynolds_.value()));
    command_sequence.push_back("\n");
  }

  if (polar_.has_value()) {
    command_sequence.push_back("oper");
    command_sequence.push_back("pacc");
    command_sequence.push_back(polar_.value());
    command_sequence.push_back("\n");
  }

  switch (mode_) {
    case Mode::ANGLE: {
      command_sequence.push_back("oper");
      command_sequence.push_back(std::string("a ") + std::to_string(aoa_.value()));
      command_sequence.push_back("\n");
      break;
    }
    case Mode::CL: {
      command_sequence.push_back("oper");
      command_sequence.push_back(std::string("cl ") + std::to_string(cl_.value()));
      command_sequence.push_back("\n");
    }
    default:
      break; 
  }

  command_sequence.push_back("quit");

  return XfoilRunner(binpath_, command_sequence); 
}

} // namespace cxxfoil

#include <cstdio>

#include "xfoil_config.h"
#include "except.h"

namespace cxxfoil {

XfoilConfig::XfoilConfig(std::string binpath) noexcept
  : binpath_(std::move(binpath)), mode_(Mode::ANGLE), aoa_(0.0)
{}

void XfoilConfig::AngleOfAttack(double aoa) noexcept {
  mode_ = Mode::ANGLE;
  aoa_ = aoa;
  cl_ = Optional<double>();
  //return std::move(*this);
}

void XfoilConfig::LiftCoefficient(double cl) noexcept {
  mode_ = Mode::CL;
  cl_ = cl;
  aoa_ = Optional<double>();
  //return std::move(*this);
}

void XfoilConfig::PaccFromString(const std::string& path) noexcept {
  polar_ = std::move(path);
  //return std::move(*this);
}

void XfoilConfig::Naca(const std::string& naca) noexcept {
  naca_ = naca;
  dat_file_ = Optional<std::string>();
  //return std::move(*this);
}

void XfoilConfig::AirfoilPolarFile(const std::string& datfile) noexcept {
  dat_file_ = datfile;
  naca_ = Optional<std::string>();
}

void XfoilConfig::PaccRandom() noexcept {
  std::string polar = std::tmpnam(nullptr);
  polar_ = polar;
  //return std::move(*this);
}

void XfoilConfig::Reynolds(size_t reynolds) noexcept {
  reynolds_ = reynolds;
}

XfoilRunner XfoilConfig::GetRunner() {
  std::vector<std::string> command_sequence{"plop", "G", "\n"}; 

  if (naca_.has_value()) {
    command_sequence.push_back(std::string("naca ") + naca_.value());
  } else if (dat_file_.has_value()) {
    command_sequence.push_back(std::string("load ") + dat_file_.value());
    command_sequence.push_back("");
  } else {
    throw XfoilException("Xfoil cannot run without an airfoil");
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

  return XfoilRunner(binpath_, command_sequence, polar_); 
}

} // namespace cxxfoil

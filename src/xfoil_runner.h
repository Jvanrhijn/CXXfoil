#ifndef _CXXFOIL_SRC_XFOIL_RUNNER_H
#define _CXXFOIL_SRC_XFOIL_RUNNER_H

#include <unordered_map>
#include <string>
#include <vector>

#include "optional.h"

namespace cxxfoil {

using polar = std::unordered_map<std::string, std::vector<double>>;

class XfoilRunner {
  public:
    XfoilRunner(std::string path, std::vector<std::string> command_sequence);

    polar Dispatch() const;

  private:
    std::string path_;
    Optional<std::string> polar_;
    std::vector<std::string> command_sequence_;

  private:
    polar ParsePolar(const std::string& path) const;
};

} // namespace cxxfoil

#endif // _CXXFOIL_SRC_XFOIL_RUNNER_H

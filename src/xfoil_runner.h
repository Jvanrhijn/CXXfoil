#ifndef _CXXFOIL_SRC_XFOIL_RUNNER_H
#define _CXXFOIL_SRC_XFOIL_RUNNER_H

#include <unordered_map>
#include <string>
#include <vector>

namespace cxxfoil {

class XfoilRunner {
  public:
    XfoilRunner() = default;

    std::unordered_map<std::string, std::vector<double>> Dispatch() const;

  private:
    void ParsePolar(const std::string& path) const;

    static void WriteToXfoil(const std::string& command);

};

} // namespace cxxfoil

#endif // _CXXFOIL_SRC_XFOIL_RUNNER_H

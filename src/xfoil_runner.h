#ifndef _CXXFOIL_SRC_XFOIL_RUNNER_H
#define _CXXFOIL_SRC_XFOIL_RUNNER_H

#include <unordered_map>
#include <string>
#include <vector>

#include "optional.h"

namespace cxxfoil {

//! Typedef for convenience. This type stores the Xfoil results table.
using polar = std::unordered_map<std::string, std::vector<double>>;

/**
 * @brief Object to dispatch an Xfoil computation.
 */
class XfoilRunner {
  friend class XfoilConfig;

  XfoilRunner(std::string path, 
              std::vector<std::string> command_sequence, 
              std::string polar);

  public:
    /**
     * @brief Dispatch the Xfoil child process compuatation.
     * @return The polar file containing a table of results, as an unordered_map.
     */
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

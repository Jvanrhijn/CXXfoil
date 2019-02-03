#ifndef _CXXFOIL_SRC_EXCEPT_H
#define _CXXFOIL_SRC_EXCEPT_H

#include <exception>

namespace cxxfoil {
  
class XfoilException : std::exception {
  public:
    explicit XfoilException(std::string message) : message_(message) {}

    virtual const char *what() const noexcept {
      return message_.c_str();
    }

  private:
      std::string message_;

}; // class XfoilException

} // namespace cxxfoil

#endif // _CXXFOIL_SRC_EXCEPT_H

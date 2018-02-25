//
// Created by jesse on 12/26/17.
//

#ifndef CPROP_TYPES_H
#define CPROP_TYPES_H

#include "include.h"

/**
 * @brief Vector of vectors, for storing xfoil calculation results
 */
class polar
{
  public:
    explicit polar(size_t lines)
      : contents(lines)
    {
      for (int i = 0; i < lines; i++) {
        contents[i].resize(5);
      }
    }

    std::vector<std::vector<double>> contents;
};

/**
 * @brief Throw this when Xfoil calculation fails to converge
 */
class ConvergenceException : public std::exception {
  public:
    const char* what() const throw() {
      return "VISCAL: Convergence Failed";
  }
};

#endif //CPROP_TYPES_H
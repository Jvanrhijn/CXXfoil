//
// Created by jesse on 12/26/17.
//

#ifndef CPROP_TYPES_H
#define CPROP_TYPES_H

#include "include.h"

/**
 * @brief Vector of vectors, for storing xfoil calculation results
 */
class polar : public std::vector<std::vector<double>> {
public:
    std::vector<std::vector<double>> contents;
    explicit polar(size_t lines);
};

/**
 * @brief Throw this when Xfoil calculation fails to converge
 */
class ConvergenceException : public std::exception {
public:
    virtual const char* what() const throw();
};

#endif //CPROP_TYPES_H
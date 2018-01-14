//
// Created by jesse on 12/26/17.
//

#include "types.h"

polar::polar(size_t lines) : contents(lines) {
    for (int i = 0; i<lines; i++) {
        contents[i].resize(5);
    }
}


const char* ConvergenceException::what() const throw() {
    return "VISCAL: Convergence failed";
}

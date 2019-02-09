# CXXfoil

[![Build Status](https://travis-ci.com/Jvanrhijn/CXXfoil.svg?branch=master)](https://travis-ci.com/Jvanrhijn/CXXfoil)

This library provides an easy to use C++ API for using Xfoil. It works by creating an
Xfoil configuration, which can be used to configure a child process running Xfoil.
This child process is then dispatched with either a lift coefficient or angle of attack
based computation. The results of the computation, in the form of the Xfoil
accumulated polar file, are then returned in an
`std::unordered_map<std::string, std::vector<double>>`. The library provides a 
convenient typedef for this (`cxxfoil::polar`).

**NOTE:** This library uses some Unix-specific headers. It has not been tested under
Windows, so I provide no guarantee that it will work on anything that is not Unix
compatible.

## Building with CMake

Add the following line to your CMakeLists.txt:

~~~cmake
add_subdirectory(CXXfoil) # or whatever directory you put the library in
~~~

Then link the library with your project:

~~~cmake
add_executable(${TARGET} your_main_file.cc)
target_link_libraries(${TARGET} cxxfoil)
~~~

## Example usage

Full usage example:

~~~cpp
#include <iostream>

#include "cxxfoil.h"

using namespace cxxfoil;

int main() {
  XfoilConfig config("/usr/local/bin/xfoil"); // or wherever your xfoil binary is
  config.Naca("0015");
  config.PaccRandom();
  config.AngleOfAttack(4.0);
  config.Reynolds(100000);
  
  XfoilRunner runner = config.GetRunner();
  polar result = runner.Dispatch();

  std::vector<double> cl = result["CL"];
  // etc, other keys are alpha, CD, CDp, CM, Top_Xtr, Bot_Xtr, Top_Itr, Bot_Itr

  std::cout << "Lift coefficient at alpha = 4.0: " << cl[0] << std::endl;
}
~~~

## Step-by-step breakdown

Create an `XfoilConfig` instance with the path to the Xfoil executable, and configure
it as desired. 

~~~cpp
XfoilConfig config("/bin/xfoil");
config.Naca("0015");
config.AngleOfAttack(4.0);
config.PaccRandom(); // generates a random file name under /tmp
~~~

Build an `XfoilRunner` instance, and dispatch the child process:

~~~cpp
XfoilRunner runner = config.GetRunner();
polar result = runner.Dispatch();
~~~

Access result from polar:

~~~cpp
std::vector<double> alpha = result["alpha"];
std::vector<double> CD = result["CD"];
~~~

You can also chain setters on the config object, but be sure to end this chain by
actually retrieving the runner, as at the end of the chain the temporary object
created by the constructor call will be dropped:

~~~cpp
polar result = XfoilConfig("/bin/xfoil")
  .Naca("0015")
  .LiftCoefficient(0.6)
  .Pacc("my_result.txt")
  .Reynolds(100000)
  .GetRunner()
  .Dispatch();
~~~

If you're doing a viscous calculation, especially for low Reynolds numbers or 
extreme angle of attack/lift coefficient, you may want to guard for a ConvergenceException:

~~~cpp
// ... setup XfoilConfig 
config.Reynolds(1000);
config.AngleOfAttack(9.0);
try {
  auto result = config.GetRunner().Dispatch();
catch (const cxxfoil::XfoilException &e) {
  std::cout << e.what() << std::endl;
}
~~~

## Contributing

I welcome any and all contributions; all I ask is to somewhat adhere to the Google C++
style guide. One caveat: I do like to use exceptions, although I am considering
introducing boost::expected.

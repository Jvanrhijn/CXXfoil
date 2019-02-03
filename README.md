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

## Example usage

Create an `XfoilConfig` instance with the path to the Xfoil executable, and configure
it as desired:

~~~cpp
cxxfoil::XfoilConfig config("/bin/xfoil")
config.NACA("0015");
config.AngleOfAttack(4.0);
config.PaccRandom(); // generates a random file name under /tmp
~~~

Build an `XfoilRunner` instance, and dispatch the child process:

~~~cpp
cxxfoil::XfoilRunner config.GetRunner();
cxxfoil::polar result = config.Dispatch();
~~~

Access result from polar:

~~~cpp
double alpha = result["alpha"];
double CD = result["CD"];
// etc, other keys are CL, CDp, CM, Top_Xtr, Bot_Xtr, Top_Itr, Bot_Itr
~~~

If you're doing a viscous calculation, especially for low Reynolds numbers, few iterations, or extreme angle of attack/lift coefficient, you may want to guard for a ConvergenceException:

~~~cpp
// ... setup XfoilConfig 
config.SetViscosity(1000);
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

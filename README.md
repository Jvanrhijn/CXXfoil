# CXXfoil

This library provides an easy-to-use C++ API for using Xfoil. It spawns a child process, to which commands are written via a pipeline. A second pipeline is used to read output from the process, which is logged to a file.

**Example usage**

Create an xfoil instance with the path to the Xfoil executable, and load a NACA airfoil:

~~~cpp
cxxfoil::Xfoil xfoil("/bin/xfoil")
xfoil.NACA("0015");
~~~

Set parameters and perform a viscous lift coefficient calculation:

~~~cpp
xfoil.SetIterations(100);
xfoil.SetViscosity(1e5);
auto result = xfoil.LiftCoefficient(0.5);
~~~

Access result from polar:

~~~cpp
double alpha = result[cxxfoil::alpha];
double CD = result[cxxfoil::CD];
// etc, other names are CL, CDp, CM, Top_Xtr, Bot_Xtr, Top_Itr, Bot_Itr
~~~

If you're doing a viscous calculation, especially for low Reynolds numbers, few iterations, or extreme angle of attack/lift coefficient, you may want to guard for a ConvergenceException:

~~~cpp
xfoil.SetViscosity(1000);
try {
  xfoil.AngleOfAttack(9.0);
catch (const std::exception &e) {
  std::cout << e.what() << std::endl; // Prints "VISCAL: Convergence failed"
}
~~~

**Contributing**

If you wish to contribute to the project, just fork the repository and create a new branch. Code style is mostly the Google C++ style guide, with exceptions allowed. Don't forget to write tests if you add a new method.

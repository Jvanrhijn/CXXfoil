# CXXfoil

This library provides an easy-to-use C++ API for using Xfoil. It spawns a child process_, to which commands are written via a pipeline. A second pipeline is used to read output_ from the process_, which is logged to a file. The library is very rudimentary right now, so the public interface is still subject to change. 

**Contributing**

If you wish to contribute to the project, just fork the repository and create a new branch. Avoid working on the master branch. Additionally, I ask that you follow the Google C++ style guide, with one exception: I do allow the use of exceptions (yes, pun intended), as Xfoil can be a fickle program to work with. 

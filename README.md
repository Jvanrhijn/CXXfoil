# Xfoil-Interface

This library provides an easy-to-use API for using Xfoil from within any C++ program. It spawns a child process, to which commands are written via a pipeline. A second pipeline is used to read output from the process, which is logged to a file. The library is very rudimentary right now, so the public interface is still subject to change. Additionally, I still have to actually adopt a coding style, so adding features is on hold until I do.

I originally started this project as part of a different project for building a propeller design program akin to OpenProp or JavaProp, but I think this part of it could be useful input its own right, so I'm open sourcing it here.

node-julia
==========

[![Build Status](https://travis-ci.org/waTeim/node-julia.svg?branch=master)](https://travis-ci.org/waTeim/node-julia)

Fast and simple access to Julia embedded in node.

# Installation

First install Julia, then

    npm install node-julia

When the module is built, the installer searches for Julia in several
standard locations starting with *julia* located on the command path. 
Is is assumed that the julia lib directory is located in a standard location
relative to where the julia executable is located.

# Use and Syntax

    julia = require('node-julia');

There are 3 functions; **eval** **exec** and **script**.

## eval

This function takes a single string argument and evaluates it like it was typed
in the Julia *REPL* and returns the result to a function callback.

### Examples

    julia.eval('rand(10,10)',function(x) 
    {
       console.log(x);
    });

This would invoke the function **rand(10,10)** and return a 10x10 2-dimentional
matrix and print the results.  Matricies are converted to Javascript arrays.

    julia.eval('e^10',function(x)
    {
       console.log(x);
    });

This would compute **exp(10)** and print the result as a Javascript *Number*.

## exec

This function takes a *string* as the first argument which is the name of
the Julia function to call followed by any number of arguments to be used as arguments
to the Julia function.  The final argument is a function callback.

### Example

From above, calculate the inverse of the matrix returned and print the result.

    julia.eval('rand(10,10)',function(x)   
    {
       julia.exec('inv',x,function(xInverse)
       {
          console.log(xInverse);
       });
    });

# Error conditions

Julia exceptions are caught and then re-thrown the in the node environment.  There
is currently a somewhat simplistic mapping in place to relate similar type exceptions.
If a Julia exception is caught that does not have a translation, the catchall is a 
generic Javascript error.

## An unknown method

    julia.exec('foo',1,2,3,function(res){});
      ^
    ReferenceError: Julia method foo is undefined
        at ReferenceError (native)
        at Object.<anonymous> (/Users/jeffw/src/nj-test3/test3.js:5:7)
        at Module._compile (module.js:449:26)
        at Object.Module._extensions..js (module.js:467:10)
        at Module.load (module.js:349:32)
        at Function.Module._load (module.js:305:12)
        at Function.Module.runMain (module.js:490:10)
        at startup (node.js:124:16)
        at node.js:807:3

## By comparison a known function, but invoked with the wrong arguments

    julia.exec('inv',1,2,3,function(res){ console.log(res); });
      ^
    ReferenceError: Julia unmatched method inv(Int64,Int64,Int64)
        at ReferenceError (native)

## A general error

    julia.eval('(*!@&$(*&@$*(!@&$(*&@',function(res){});
      ^
    Error: Julia syntax: missing separator in tuple
        at Error (native)

# Tests
Tests have been added to the directory test, and can be run using npm

    npm test

# Limitations

The following are limitations of the current framework, and while all are planned to
be addressed in the future, the implementation is difficult enough that it was thought
better to provide the limited form now rather than wait several weeks or more for the
full version.  There are various workaround for most of these issues.

* Currently only primitative types and arrays of primatives are supported.  Composites
are planned.  As a workarond use Julia composites within a module or wrapping function
and export the result to node as a tuple which will be mapped to seperate
function callback variables.

* Julia invocations are currently synchronous.  For although the underlying top and 
bottom parts of the framework operate in separate threads, the top half simply blocks
until the bottom half is finished.  Asynchronicity will be supported in a upcoming 
release.

## OS specific issues.

### Linux
Currently, supported linux installations include any version that uses the binary
distribution of Julia (e.g. ubuntu PPA), or if Julia is compiled from source, then
the nodejs version 11.13 is compatible since both Julia and node use the same version
of libuv.

### Windows
Work on Windows support is in progress.

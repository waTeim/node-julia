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

# Sample Syntax

To compute the solution to a system of 3 equations and 3 unknowns.

    julia = require('node-julia');

    var a = julia.eval('[1 2 3; 4 5 6; 7 8 9 ]');
    var b = julia.eval("[1 1 1]'");
    var c = julia.exec('\\',a,b);

    console.log('Solution: ' + c[0] + 'x + ' + c[1] + 'y + ' + c[2] + 'z');

# A Simple API

There are 3 functions; **eval** **exec** and **Script**, [see here](http://node-julia.readme.io/)
for full documentation.

## eval

This function takes a single string argument and evaluates it like it was typed
in the Julia *REPL* and returns the result

    julia.eval('e^10', function(x) {
       console.log('exp(10) = ' + x);
    });

Calls to **eval** without a function callback are also supported. Matrices 
are easily constructed using Julia's Matlab-like matrix syntax.

    console.log("2x2 matrix: ", julia.eval('[ 1 2; 3 4]'));

## exec

This function takes a *String* the identifies the Julia function to
use followed by any number of arguments for that function.  Like **eval**
the last argument may be a function callback.

Calculate the inverse of a matrix and print the result.

    var a = julia.eval('[ 2 1; 1 1]');
    
    julia.exec('inv',a,function(xInv)
    {
       console.log("Inverse: " + xInv);
    });

## Script

Julia scripts can be functionalized and compiled and then subsequently
called using **Script.exec** which has the same semantics as **exec**.

    var aScript = julia.Script('ascript.jl');

    aScript.exec();

## Error conditions

Julia exceptions are caught and then re-thrown the in the node environment.

# Tests
Tests run using npm

    npm test

# Limitations

* Currently only primitative types and arrays of primatives are supported.

* Julia invocations are currently synchronous.

* Currently, supported linux installations include any version that uses the binary
distribution of Julia (e.g. ubuntu PPA), or if Julia is compiled from source, then
the nodejs version 11.13.

* Work on Windows support is in progress.

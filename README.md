node-julia
==========

[![Build Status](https://travis-ci.org/waTeim/node-julia.svg?branch=master)](https://travis-ci.org/waTeim/node-julia)

Fast and simple access to Julia embedded in node.

# Installation

First install [Julia](http://julialang.org/), then

    npm install node-julia

When the module is built, the installer searches for Julia in several
standard locations starting with **julia** located on the command path.
Is is assumed that the julia lib directory is located in a standard location
relative to where the julia executable is located [see here](http://node-julia.readme.io/)
for full documentation and release notes.

# Sample Syntax

To compute the solution to a system of 3 equations and 3 unknowns, use the Julia
linear algebra package.

    julia = require('node-julia');

    var a = julia.eval('[ 1 2 3 ; 3 4 1; 8 2 9]');
    var b = julia.eval("[1 1 1]'");
    var c = julia.exec('\\',a,b);

    console.log('Solution: ' + c[0][0] + 'x + ' + c[1][0] + 'y + ' + c[2][0] + 'z');

produces

    Solution: -0.1463414634146341x + 0.3170731707317073y + 0.17073170731707316z

# A Simple API

There are 4 base functions; **eval** **exec** and **Script**, and *import*.

## eval

This function takes a single string argument and evaluates it like it was typed
in the Julia command line and returns the result *res* while *err* will be false if
successful or contain a error message otherwise

    julia.eval('e^10',function(err,res) {
       if(!err) console.log('exp(10) = ',res)
    });

    // simple prevention of premature exit
    setTimeout(function(){ console.log('done.'); },1000);

Calls to **eval** without a function callback are also supported. Matrices
are easily constructed using Julia's Matlab-like matrix syntax.

    console.log('2x2 matrix: ',julia.eval('[1 2; 3 4]'));

## exec

This function takes a *String* naming the Julia function to use followed by any number of
 arguments for that function.  Like **eval** the last argument may be a function callback.

Calculate the inverse of a matrix and print the result.

    var a = julia.eval('[2 1; 1 1]');

    julia.exec('inv',a,function(err,inv) {
       if(!err) {
          console.log("Inverse is:");
          for(var i = 0;i < 2;i++)
             console.log('[' + inv[i][0] + ', ' + inv[i][1] + ']');
       }
       else console.log(err);
    });
    setTimeout(function(){ console.log('done.'); },1000);

## import

Julia modules may be imported using import and the functions exported by
the module will be mapped to Javascript functions. For example,
[JuMP](http://jump.readthedocs.org/en/latest) can conveniently be
used to solve a linear programming problem in the following way:

    var julia = require('node-julia');
    var JuMP = julia.import('JuMP');
    var m = julia.eval('m = JuMP.Model()');
    var m = julia.eval('m = JuMP.Model()');
    var x = julia.eval('JuMP.@defVar(m,0 <= x <= 2)');
    var y = julia.eval('JuMP.@defVar(m,0 <= y <= 30)');

    julia.eval('JuMP.@setObjective(m,Max, 5x + 3*y)');
    julia.eval('JuMP.@addConstraint(m,1x + 5y <= 3.0)');

    var status = JuMP.solve(m);

    console.log('Objective value: ',JuMP.getObjectiveValue(m));
    console.log('X value: ',JuMP.getValue(x));
    console.log('Y value: ',JuMP.getValue(y));

## Script

Julia scripts can be functionalized and compiled and then subsequently
called using **Script.exec** which has the same semantics as **exec**.

        var aScript = julia.Script('ascript.jl');

        aScript.exec();

# Synchronous and Asynchronous Calls
Both synchronous and asynchronous calls are supported and the type used is
indicated by the presence (or lack) of a callback function.

    var a = julia.exec('rand',400,400);          // synchronous

    julia.exec('svd',a,function(err,u,s,v) {     // asynchronous
    ...
    });

# Error Conditions

When executing a call synchronously, Julia errors are caught and then
thrown as JavaScript exceptions.  Conversely, when Julia errors occur when
processing asynchronously, the error code is returned as the first argument
to the callback function.

# Use of JavaScript Typed Arrays
Typed arrays are used when possible which will be whenever the element
type is numeric in either single dimension or multidimensional arrays

## Additionally
* Javascript **Buffer** will be mapped to Julia **UInt8** Arrays
* Julia single dimension **UInt8** arrays will be mapped to **Buffer**, but
Julia multidimensional **UInt8** arrays will be mapped to JavaScript Arrays
of **UInt8Array**.
* Julia **UInt64** and **Int64** arrays will be mapped to JavaScript
**Float64Array**.

## Use of Shared Buffers
If the result of an expression would resolve to a JavaScript typed array, then
the buffer that underlies that array is shared, not copied, between Julia and JavaScript.
* This feature is currently not avaiable when using node 0.10.
* This feature is currently supported on all other versions of node and iojs
* If the array originates within Julia, then a Javascript typed array will be
created as described above with the same buffer used by Julia.
* If the array originates within JavaScript then an equivalent Julia array will be created
by reusing the same memory buffer as the JavaScript array.
* Reuse of the same array across multiple calls is detected to prevent re-creation.
* Premature garbage collection is prevented.

# Tests
Tests run using npm

    npm test

# Compatibility
Tested with [node](http://nodejs.org/) 0.10, 0.11, 0.12, and 4.0 as well as
[io.js](https://iojs.org/) 1.x, 2.x. 3.x.

Julia version 0.3 is supported on all versions of node and iojs, but Julia 0.4+
and node 0.10 are incompatible.

Tested on OS/X, Linux, Windows.

# Limitations

* node 0.10 is deprecated due to Julia changes that have caused an incompatibility
between Julia 0.4 and node 0.10.

node-julia
==========

A module linking node.js and Julia.

#Installation#

Rather than installing Julia along with this module, it is assumed that
the Julia install has already occured.  This is appropriate since Julia
has it's own package management system as well as other tools that it
interacts with; having it install as a package subordinate to *node-julia*
was thought too restrictive for the minor ease of installation that 
would provide.

So, install Julia first.

When this module is built, the script *tools/find_julia.py* is invoked which
searches for *Julia* in several standard locations starting with **julia**
located on the command path.  Is is assumed that the *Julia* install directory
structure is the standard one with (importantly) the Julia *lib* directory
located in a standard location relative to the *bin* directory where the
**julia** executable is located. It is this lib driectory that contains the
necessary embedding API that comes with the language.

#Use and Syntax#

    julia = require('node-julia');

Currently, there are 3 exported functions; **start**, **eval**, and **exec**.  

##start##

This function starts the embedded julia engine.  It need be called only once,
but can be called multiple times, but will have no effect after the first time.
It must be called before **eval** or **exec** is called.  It may be removed
in the future and called implicitly.  It takes a single optional argument,
the location of the *Julia* installation which will otherwise be assumed to be
location of Julia that was detected when **node-julia** was installed.

###Example###

   julia.start();     // Uses default location

or alternately

   julia.start('/usr/local/julia/');  // overrides standard location

##eval##

This function takes a single string argument and evaluates it like it was typed
in the *Julia* REPL and returns the result to a function callback.

###Examples###

   julia.eval('rand(10,10)',function(x) 
   {
      console.log(x);
   });

This would invoke the function **rand**(10,10) and return a 10x10 2-dimentional
matrix and print the results.  Matricies are converted to *Javascript* Arrays.

   julia.eval('e^10',function(x)
   {
      console.log(x);
   });

This would compute exp(10) and print the result as a *Javascript* **Number**.

##exec##

This function takes a **String** as the first argument indicating the name of
the *Julia* function to call followed by any number of arguments to be used as arguments
to the *Julia* function.  The final argument is a function callback.

###Example##

From above, calculate the inverse of the matrix returned and print the result.

   julia.eval('rand(10,10)',function(x)   
   {
      julia.exec('inv',x,function(xInverse)
      {
         console.log(xInverse);
      });
   });


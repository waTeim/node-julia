var x = require("bindings")("nj.node");

function JuliaModule(moduleRef,functionNames)
{
   this.moduleRef = moduleRef;
   this.functionNames = functionNames;

   for(var i = 0;i < functionNames.length;i++)
      this[functionNames[i]] = this.thunk(functionNames[i]);
}

JuliaModule.prototype.thunk = function(functionName)
{
   var commonArgs = [this.moduleRef,functionName];

   return function()
   {
      var argv = [commonArgs[0],commonArgs[1]];

      for(var i = 0;i < arguments.length;i++) argv.push(arguments[i]);
      return x.exec.apply(x,argv);
   }
}

module.exports =
{
   eval:x.eval,
   exec:x.exec,
   import:function(juliaModuleName,callback)
   {
      if(callback)
      {
         x.import(juliaModuleName,function(err,m,functionNames)
         {
            if(err) callback(err);
            else
            {
               var moduleObj = new JuliaModule(m,functionNames);

               callback(null,moduleObj);
            }
         });
      }
      else
      {
         var res = x.import(juliaModuleName);

         return new JuliaModule(res[0],res[1]);
      }
   },
   Script:x.Script,
   newScript:x.newScript
};

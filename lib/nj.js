var x = require("bindings")("nj.node");

module.exports =
{
   start:x.start,
   eval:x.eval,
   exec:x.exec,
   Script:x.Script,
   newScript:x.newScript
};

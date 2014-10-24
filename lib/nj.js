var x = require("bindings")("nj.node");

module.exports =
{
   eval:x.eval,
   exec:x.exec,
   Script:x.Script,
   newScript:x.newScript
};

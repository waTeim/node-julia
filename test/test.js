var chai = require('chai'),
    julia = require('..'),
    expect = chai.expect;

function eval(julia,x)
{
   var res;

   julia.eval('' + x,function(juliaResult)
   {
      res = juliaResult; 
   });

   return res;
}

function execIdentity(julia,x)
{
   var res;

   julia.exec('identity',x,function(juliaResult)
   {
      res = juliaResult; 
   });

   return res;
}

function execInclude(julia,filename)
{
   var res;

   julia.exec('include',filename,function(juliaResult)
   {
      res = juliaResult;
   });

   return res;
}

function execA(julia)
{
   var args = [];
   var res;

   for(var i = 1;i < arguments.length;i++) args.push(arguments[i]);
   args.push(function(juliaResult) { res = juliaResult; });

   julia.exec.apply(null,args);

   return res;
}

function execB(script)
{
   var args = [];
   var res;

   for(var i = 1;i < arguments.length;i++) args.push(arguments[i]);
   args.push(function(juliaResult) { res = juliaResult; });

   script.exec.apply(script,args);

   return res;
}

function verifyIdentity(X,expectedLength)
{
   var len = X.length;
   var passed = X.length == expectedLength;

   if(!passed) return false;

   for(var i = 0;i < X.length;i++)
   {
      if(X[i].length != X.length) 
      {
         passed = false;
         break;
      }

      for(j = 0;j < len;j++)
      {
         if(i == j && X[i][j] != 1)
         {
            passed = false;
            break;
         }
         if(i != j && X[i][j] != 0)
         {
            passd = false;
            break;
         }
      }
   }

   return passed;
}

describe('Regression Tests',function()
{
   it('eval Null return',function()
   {
      expect(eval(julia,"()")).to.equal(null);
   });

   it('eval Boolean return',function()
   {
      expect(eval(julia,true)).to.equal(true);
   });

   it('eval Integer return',function()
   {
      expect(eval(julia,0)).to.equal(0);
   });

   it('eval max 32 bit Integer (4294967296)',function()
   {
      expect(eval(julia,4294967296)).to.equal(4294967296);
   });

   it('eval max JavaScript Integer (9007199254740992)',function()
   {
      expect(eval(julia,9007199254740992)).to.equal(9007199254740992);
   });

   it('eval primitive Float return',function()
   {
      expect(eval(julia,1.0)).to.equal(1.0);
   });

   it('eval max JavaScript Float (' + Number.MAX_VALUE + ')',function()
   {
      expect(eval(julia,Number.MAX_VALUE)).to.equal(Number.MAX_VALUE);
   });

   it('simple result return style for Eval',function()
   {
      expect(julia.eval('"x"')).to.equal('x');
   });

   it('tuple result return style for Eval',function()
   {
      expect(julia.eval('("x","y")')).to.eql(['x','y']);
   });

   it('exec identity Null',function()
   {
      expect(execIdentity(julia,null)).to.equal(null);
   });

   it('exec identity Boolean',function()
   {
      expect(execIdentity(julia,true)).to.equal(true);
   });

   it('exec identity Integer',function()
   {
      expect(execIdentity(julia,1)).to.equal(1);
   });

   it('exec identity Float',function()
   {
      expect(execIdentity(julia,0.1)).to.equal(0.1);
   });

   it('exec identity String',function()
   {
      expect(execIdentity(julia,'x')).to.equal('x');
   });

   it('simple result return style for Exec',function()
   {
      expect(julia.exec('identity','x')).to.equal('x');
   });

   it('implicit conversion from SubString to String',function()
   {
      expect(eval(julia,'match(r"(a)","a").match')).to.equal("a");
   });

   it('eval include',function()
   {
      expect(eval(julia,'Core.include("test/inc1.jl")')).to.equal(true);
   });

   it('exec include',function()
   {
      expect(execInclude(julia,'test/inc2.jl')).to.equal(true);
   });

   it('user defined functions via exec',function()
   {
      expect(execA(julia,'f',100)).to.equal(execA(julia,'g',100));
   });

   it('macros via eval',function()
   {
      expect(eval(julia,'@sprintf("x")')).to.equal('x');
   });

   it('arrays via eval',function()
   {
      expect(verifyIdentity(eval(julia,'eye(1000)'),1000)).to.equal(true);
   });

   it('arrays via exec',function()
   {
      expect(verifyIdentity(execA(julia,'eye',1000),1000)).to.equal(true);
   });

   it('construction of Script using new',function()
   {
      var script = new julia.Script("test/inc1.jl");

      expect(script.getPath()).to.equal('test/inc1.jl');
   });

   it('construction of Script using function call like syntax',function()
   {
      var script = julia.Script("test/inc2.jl");

      expect(script.getModuleName()).to.equal('njIsoMod1');
   });

   it('script creation (via factory)',function()
   {
      var script = julia.newScript("test/inc3.jl");

      expect(verifyIdentity(execB(script,10),10)).to.equal(true);
   });

   it('script exec using return style',function()
   {
      var script = julia.newScript("test/inc3.jl");

      expect(script.exec(5,6)).to.eql(julia.exec('eye',5,6));
   });

   it('loading array type tests',function()
   {
      expect(execInclude(julia,'test/inc4.jl')).to.equal(true);
   });

   it('Typecheck Null array elements',function()
   {
      expect(execA(julia,'typecheckArray',[null])).to.equal('void');
   });

   it('Typecheck Boolean array elements',function()
   {
      expect(execA(julia,'typecheckArray',[true,false,true])).to.equal('boolean');
   });

   it('Typecheck Integer array elements',function()
   {
      expect(execA(julia,'typecheckArray',[1.0])).to.equal('int');
   });

   it('Typecheck Float array elements',function()
   {
      expect(execA(julia,'typecheckArray',[1.1])).to.equal('float');
   });

   it('Typecheck String array elements',function()
   {
      expect(execA(julia,'typecheckArray',[""])).to.equal('string');
   });

   it('Typecheck [Boolean,Integer] -> [Integer]',function()
   {
      expect(execA(julia,'typecheckArray',[true,1])).to.equal('int');
   });

   it('Typecheck [Boolean,Float] -> [Float]',function()
   {
      expect(execA(julia,'typecheckArray',[true,1.1])).to.equal('float');
   });

   it('Typecheck [Boolean,String] -> [String]',function()
   {
      expect(execA(julia,'typecheckArray',[""])).to.equal('string');
   });

   it('Typecheck [Integer,Float] -> [Float]',function()
   {
      expect(execA(julia,'typecheckArray',[1,1.1])).to.equal('float');
   });

   it('Prevent widening String elements',function()
   {
      expect(execA(julia,'typecheckArray',[true,"x",1,1.1])).to.equal('none');
   });

   it('Prevent widening Null elements',function()
   {
      expect(execA(julia,'typecheckArray',[true,null,1,1.1])).to.equal('none');
   });

   it('Simple Integer array input',function()
   {
      expect(execA(julia,'sum',[1,2,3])).to.equal(6);
   });

   it('Simple Float array input',function()
   {
      expect(execA(julia,'sum',[1.5,2.6,3.7])).to.equal(7.8);
   });

   it('Simple String array input',function()
   {
      expect(execA(julia,'concat',['a','b','c'])).to.equal('abc');
   });

   it('Array of null',function()
   {
      expect(eval(julia,'Array(Void,2,2)')).to.eql([[null,null],[null,null]]);
   });

   it('Array of elementwise conversion from SubString to String',function()
   {
      expect(julia.exec('split','a b c',' ')).to.eql(['a','b','c']);
   });
});


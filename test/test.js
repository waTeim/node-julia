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

   it('version',function()
   {
      expect(julia.eval('VERSION.minor')).to.within(3,4);
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
      // Keep Core.include for the time being.  This should be unnecessary, but
      // currently is.  Once the libuv problems are resolved, Core.include
      // can be shortened to simply include.
      expect(julia.eval('Core.include("test/inc1.jl")')).to.equal(true);
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

   it('Native Int8Array to Array{Int8,1}',function()
   {
      var a = new Int8Array(256);

      for(var i = 0;i < 256;i++) a[i] = i - 128;
      
      expect(julia.exec('sum',a)).to.equal(-128);
   });

   it('Native Uint8Array to Array{Uint8,1}',function()
   {
      var a = new Uint8Array(256);

      for(var i = 0;i < 256;i++) a[i] = i;

      expect(julia.exec('sum',a)).to.equal(32640);
   });

   it('Native Int16Array to Array{Int16,1}',function()
   {
      var a = new Int16Array(65536);

      for(var i = 0;i < 65536;i++) a[i] = i - 32768;

      expect(julia.exec('sum',a)).to.equal(-32768);
   });

   it('Native Uint16Array to Array{Uint16,1}',function()
   {
      var a = new Uint16Array(65536);

      for(var i = 0;i < 65536;i++) a[i] = i;
      
      expect(julia.exec('sum',a)).to.equal(2147450880);
   });

   it('Native Int32Array to Array{Int32,1}',function()
   {
      var a = new Int32Array(100000);

      for(var i = 0;i < 100000;i++)
      {  
         if(i % 2 == 0) a[i] = i - 2147483648;
         else a[i] = 2147483647 - i;
      }
      
      expect(julia.exec('sum',a)).to.equal(-100000);
   });

   it('Native Uint32Array to Array{Uint32,1}',function()
   {
      var a = new Uint32Array(100000);

      for(var i = 0;i < 100000;i++) a[i] = 4294967295 - i;

      expect(julia.exec('sum',a)).to.equal(429491729550000);
   });

   it('Native Float32Array to Array{Float32,1}',function()
   {
      var a = new Float32Array(10000);

      for(var i = 0;i < 10000;i++) a[i] = 123.45678;

      expect(Math.abs(julia.exec('sum',a) - 1.2345678E6)).to.be.below(50);
   });

   it('Native Float64Array to Array{Float64,1}',function()
   {
      var a = new Float64Array(10000);

      for(var i = 0;i < 10000;i++) a[i] = 1.2345678E39

      expect(Math.abs(julia.exec('sum',a) - 1.2345678E43)).to.be.below(1E30);
   });

   it('Buffer to Array{Uint8,1} and back as Buffer',function()
   {
      var b = new Buffer(400000);

      for(var i = 0;i < 400000;i++) b[i] = i % 256;
      expect(julia.exec('identity',b)).to.eql(b);
   });

   it('Buffer to Array{Uint8,1} and back as reshaped Array',function()
   {
      var b = new Buffer(6);

      for(var i = 0;i < b.length;i++) b[i] = i;
      expect(julia.exec('reshape',b,2,3)).to.eql([[0,2,4],[1,3,5]]);
   });

   it('Multidimensional Array',function()
   {
      var a = [ [ [1,2], [3,4] ], [ [5,6], [7,8] ] ]

      expect(julia.exec('identity',a)).to.eql(a);
   });

   it('Buffer to Multidimensional Array',function()
   {
      var b = new Buffer(16);

      for(var i = 0;i < b.length;i++) b[i] = i;

      expect(julia.exec('reshape',b,2,2,2,2)).to.eql([ [[[0, 8], [4, 12]], [[2, 10], [6, 14]]],[[[1, 9], [5, 13]], [[3, 11], [7, 15]]]]);
   });

   it('Simplistic Regex',function()
   {
      var re = /a/;

      expect(julia.exec('identity',re)).to.eql(re);
   });

   /***
    *
    * Kind of a workaround here
    *
    */
   it('Date (Julia version 0.4+ only)',function()
   {
      var now = new Date();
      var version = julia.eval('VERSION.minor');

      if(version == 4) expect(julia.exec('identity',now)).to.eql(now);
   });
});

module nj

macro vers03x(ex)
   VERSION.minor == 3
end

macro vers03x_only(ex)
   @vers03x(ex)?esc(ex):nothing
end

macro vers04x(ex)
   VERSION.minor == 4
end

macro vers04x_only(ex)
   @vers04x(ex)?esc(ex):nothing
end

function topExpr(mod::Module,paths::Array{ASCIIString,1})
   res = Expr(:toplevel,:(eval(x) = Core.eval(mod,x)),:(eval(m,x) = Core.eval(m,x)))
   for path in paths
      push!(res.args,:(include($path)))
   end
   return res
end

include(mod::Module,path,args::Vector) = include(mod,path,UTF8String[args...])

function scriptify(mod::Module,filename::ASCIIString)
    ast = parse("function _(args...)\n" * readall(filename) * "end");
    args2 = Array(Any,0);
    paths = Array(ASCIIString,0);

    for astNode in ast.args[2].args
       if(typeof(astNode) == Expr && astNode.head == :call && astNode.args[1] == :include) 
          push!(paths,astNode.args[2]);
       else
          push!(args2,astNode);
       end
    end
    res = topExpr(mod,paths);
    push!(res.args,Expr(ast.head,ast.args[1],Expr(ast.args[2].head,args2...)));
    res
end

newRegex(pattern) = Regex(pattern)
getPattern(re::Regex) = re.pattern
getRegexType() = Regex

@vers04x_only getDateTimeType() = DateTime
@vers03x_only getDateTimeType() = typeof(nothing)

end

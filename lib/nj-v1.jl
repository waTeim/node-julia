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

macro vers04orGreater(ex)
   VERSION.minor >= 4
end

macro vers04orGreater_only(ex)
   @vers04orGreater(ex)?esc(ex):nothing
end

macro vers04orLess(ex)
   VERSION.minor <= 4
end

macro vers04orLess_only(ex)
   @vers04orLess(ex)?esc(ex):nothing
end

macro vers05x(ex)
   VERSION.minor == 5
end

macro vers05x_only(ex)
   @vers05x(ex)?esc(ex):nothing
end

# Something more parametric.  Work around the problem of julia version 0.3
# not having a VersionNumber constructor by always returning true
macro vLT(v,ex)
   VERSION.minor <= 3 || VERSION < VersionNumber(v)?esc(ex):nothing
end

# Likewise as above always return false for version minor <= 3 and only
# differentiate for later versions.
macro vGE(v,ex)
   VERSION.minor > 3 && VERSION >= VersionNumber(v)?esc(ex):nothing
end

# lifted from Compat.jl; minimum required
macro compat(ex)
    esc(_compat(ex))
end

function _compat(ex::Expr)
    if ex.head == :curly
        f = ex.args[1]
        if VERSION < v"0.4.0-dev+5379" && f == :Union
            ex = Expr(:call,:Union,ex.args[2:end]...)
        end
    elseif ex.head == :quote && isa(ex.args[1], Symbol)
        # Passthrough
        return ex
    end
    return Expr(ex.head, map(_compat, ex.args)...)
end
function _compat(ex::Symbol)
    if VERSION < v"0.4.0-dev+768" && ex == :Void
        return :Nothing
    end
    return ex
end
_compat(ex) = ex

if VERSION < v"0.4.0-dev+1387"
    typealias AbstractString String
    export AbstractString
end

#--------- end compat.jl lifting ----------

preserve = Array(Any,0);

function topExpr(mod::Module,paths::Array{ASCIIString,1})
   res = Expr(:toplevel,:(eval(x) = Core.eval(mod,x)),:(eval(m,x) = Core.eval(m,x)))
   for path in paths
      push!(res.args,:(include($path)))
   end
   return res;
end

include(mod::Module,path,args::Vector) = include(mod,path,UTF8String[args...])
@vLT "0.5.0-dev+2300" readAllFromFile(filename) = readall(filename)
@vGE "0.5.0-dev+2300" readAllFromFile(filename) = readstring(filename)

function scriptify(mod::Module,filename::ASCIIString)
    ast = parse("function _(args...)\n" * readAllFromFile(filename) * "end");
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
    return res;
end

newRegex(pattern) = Regex(pattern)
getPattern(re::Regex) = re.pattern
getRegexType() = Regex

@vers03x_only getDateTimeType() = typeof(nothing)
@vers04orGreater_only getDateTimeType() = DateTime
@vers03x_only toDate(milliseconds) = nothing

@vers04orGreater_only function toDate(milliseconds::Int64)
   return DateTime(1970) + Base.Dates.Millisecond(milliseconds)
end

@vers03x_only toMilliseconds(date) = nothing
@vers04orGreater_only toMilliseconds(date) = Base.Dates.datetime2unix(date);

function getError(ex,bt)
   io = IOBuffer();
   Base.showerror(io,ex,bt);
   s = takebuf_string(io);
   return s;
end



#  The following copies the base definition of require and the supporting functions
#  but redefines pwd() to avoid the conflict caused by node's and julia's double
#  definition of uv_cwd which are not compatible.
#
#  This can be removed once the issue is resolved in base.

@vers04orGreater_only function pwd()
    b = Array(UInt8,1024)
    len = Csize_t[length(b),]
    Base.uv_error(:getcwd, ccall((:uv_cwd,"libjulia"), Cint, (Ptr{UInt8}, Ptr{Csize_t}), b, len))
    bytestring(b[1:len[1]-1])
end

@vers03x_only function pwd()
    b = Array(Uint8,1024)
    len = Csize_t[length(b),]
    Base.uv_error(:getcwd, ccall((:uv_cwd,"libjulia"), Cint, (Ptr{Uint8}, Ptr{Csize_t}), b, len))
    bytestring(b[1:len[1]-1])
end

abspath(a::AbstractString) = normpath(isabspath(a) ? a : joinpath(pwd(),a))
abspath(a::AbstractString, b::AbstractString...) = abspath(joinpath(a,b...))

function find_in_path(name::AbstractString)
    isabspath(name) && return name
    isfile(name) && return abspath(name)
    base = name
    if endswith(name,".jl")
        base = name[1:end-3] # this is valid because ".jl" is representable in 3 bytes
    else
        name = string(base,".jl")
        isfile(name) && return abspath(name)
    end
    for prefix in [Pkg.dir(); LOAD_PATH]
        path = joinpath(prefix, name)
        isfile(path) && return abspath(path)
        path = joinpath(prefix, base, "src", name)
        isfile(path) && return abspath(path)
        path = joinpath(prefix, name, "src", name)
        isfile(path) && return abspath(path)
    end
    return nothing
end

find_in_node1_path(name) = myid()==1 ?
    find_in_path(name) : remotecall_fetch(1, find_in_path, name)

function find_source_file(file)
    (isabspath(file) || isfile(file)) && return file
    file2 = find_in_path(file)
    file2 != nothing && return file2
    file2 = joinpath(JULIA_HOME, DATAROOTDIR, "julia", "base", file)
    isfile(file2) ? file2 : nothing
end

# Store list of files and their load time
package_list = Dict{ByteString,Float64}()
# to synchronize multiple tasks trying to require something
package_locks = Dict{ByteString,Any}()
#require(fname::AbstractString) = require(bytestring(fname))
require(f::AbstractString, fs::AbstractString...) = (require(f); for x in fs require(x); end)

# only broadcast top-level (not nested) requires and reloads
toplevel_load = true

function require(name::AbstractString)
    path = find_in_node1_path(name)
    path == nothing && throw(ArgumentError("$name not found in path"))

    if myid() == 1 && toplevel_load
        refs = Any[ @spawnat p _require(path) for p in filter(x->x!=1, procs()) ]
        _require(path)
        for r in refs; wait(r); end
    else
        _require(path)
    end
    nothing
end

function _require(path)
    global toplevel_load
    if haskey(package_list,path)
        wait(package_locks[path])
    else
        last = toplevel_load
        toplevel_load = false
        try
            reload_path(path)
        finally
            toplevel_load = last
        end
    end
end

function reload(name::AbstractString)
    global toplevel_load
    path = find_in_node1_path(name)
    path == nothing && throw(ArgumentError("$name not found in path"))
    refs = nothing
    if myid() == 1 && toplevel_load
        refs = Any[ @spawnat p reload_path(path) for p in filter(x->x!=1, procs()) ]
    end
    last = toplevel_load
    toplevel_load = false
    try
        reload_path(path)
    finally
        toplevel_load = last
    end
    if refs !== nothing
        for r in refs; wait(r); end
    end
    nothing
end

# remote/parallel load

function source_path(default::@compat(Union{AbstractString,Void})="")
    t = current_task()
    while true
        s = t.storage
        if !is(s, nothing) && haskey(s, :SOURCE_PATH)
            return s[:SOURCE_PATH]
        end
        if is(t, t.parent)
            return default
        end
        t = t.parent
    end
end

macro __FILE__() source_path() end

function include_from_node1(path::AbstractString)
    prev = source_path(nothing)
    path = (prev == nothing) ? abspath(path) : joinpath(dirname(prev),path)
    tls = task_local_storage()
    tls[:SOURCE_PATH] = path
    local result
    try
        if myid()==1
            # sleep a bit to process file requests from other nodes
            nprocs()>1 && sleep(0.005)
            result = Core.include(path)
            nprocs()>1 && sleep(0.005)
        else
            result = include_string(remotecall_fetch(1, readall, path), path)
        end
    finally
        if prev == nothing
            delete!(tls, :SOURCE_PATH)
        else
            tls[:SOURCE_PATH] = prev
        end
    end
    result
end

@vers04orLess_only function getReference()
   if :RemoteRef in names(Base)
      return RemoteRef()
   else
      return RemoteChannel()
   end
end

@vers05x_only function getReference()
   return RemoteChannel()
end

function reload_path(path::AbstractString)
    had = haskey(package_list, path)
    if !had
       package_locks[path] = getReference()
    end
    package_list[path] = time()
    tls = task_local_storage()
    prev = pop!(tls, :SOURCE_PATH, nothing)
    try
        eval(Main, :(Base.include_from_node1($path)))
    catch e
        had || delete!(package_list, path)
        rethrow(e)
    finally
        if prev != nothing
            tls[:SOURCE_PATH] = prev
        end
    end
    if !isready(package_locks[path])
        put!(package_locks[path],nothing)
    end
    nothing
end

#
# End loading.jl inclusion



function importModule(modulePath::ASCIIString)
   separator = "/";
   try
      separator = Base.path_separator
   catch(e)
      separator = Base.Filesystem.path_separator
   end
   substitutedModulePath= reduce((x,y)->"$x$(separator)$y",split(modulePath,"/"))
   require(substitutedModulePath);

   functionNames::Array{UTF8String,1} = Array(UTF8String,0);
   m::Module = Base.eval(Main,symbol(split(substitutedModulePath,separator)[end]));

   for name in names(m)
      push!(functionNames,"$name");
   end
   return m,functionNames;
end

newTuple(args...)=tuple(args...)

end

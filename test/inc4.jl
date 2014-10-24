typecheckArray()="none"
typecheckArray(x::Array{Bool,1})="boolean"
typecheckArray(x::Array{Int64,1})="int"
typecheckArray(x::Array{Float64,1})="float"
typecheckArray(x::Array{UTF8String,1})="string"
concat(x::Array{UTF8String,1})=reduce((x,y)->x*y,x)
true

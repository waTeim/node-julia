type T1
   f1::Int64
   f2::Array{Float64,1}
end

function t1Cons1(a::Int64,n::Int64)
   T1(a,[i for i=1:n])
end

function t1Cons2(a::Int64,n::Int64)
   T1(a,rand(n))
end

function t1Mult(t::T1)
   return t.f1*t.f2;
end

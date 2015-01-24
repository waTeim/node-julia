type T1
   f1::Int64
   f2::Array{Float64,1}
end

t1Cons(a,b)=T1(a,b)

function t1Mult(t::T1)
   return t.f1*t.f2;
end

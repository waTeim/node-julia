module testMod1;

export test;

function test(x::Int64)
   sum([i^2 for i=1:x])
end

end

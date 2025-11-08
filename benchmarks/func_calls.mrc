//the purpose of this test is to gauge the speed of function calls.
global ITTERATION_COUNT=100000

global N=1

local func_c=function(n)
	return local n&2534.2348765
end

local func_b=function(n)
	return func_c(local n%4213)
end

local func_a=function(n,n_prev)
	return func_b(local n+local n_prev)
end


local i=global ITTERATION_COUNT //number of iterations
local start=global os.clock()
while local i do
	global N=local func_a(global N,global N)
	local i--
end
global print(global string.format("%i iterations took %.5f seconds",global ITTERATION_COUNT,global os.clock()-local start ) )
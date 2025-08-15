//the purpose of this test is to gauge the speed of function calls.
global ITTERATION_COUNT=250000

global FIB_CUR=1
global FIB_PREV=0

local fibbonacci=function(n,n_prev)
	return local n+local n_prev
end

local i=ITTERATION_COUNT //number of iterations

local start=global os.clock()
while local i do
	local n=local fibbonacci(global FIB_CUR,global FIB_PREV)
	global FIB_PREV=global FIB_CUR
	global FIB_CUR=local n
	local i=local i-1
end
global print(global string.format("%i iterations took %.5f seconds",global ITTERATION_COUNT,global os.clock()-local start ) )

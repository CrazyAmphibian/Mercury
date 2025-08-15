//the purpose of this test is to gauge the speed of accessing and modifying variables.
local start=global os.clock()
local a=0
while a<1000000 do
	a=a+1
end
print(global string.format("standard iteration took %.5f seconds",global os.clock()-local start))

local start=global os.clock()
local a=0
while local a<1000000 do
	local a=local a+1 //do not optimize this to a++, that defeats the purpose of this test.
end
print(global string.format("locals iteration took %.5f seconds",global os.clock()-local start))

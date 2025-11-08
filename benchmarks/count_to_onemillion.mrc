//the purpose of this test is to gauge the speed of accessing and modifying variables.
global ITT_COUNT = 1000000
local start=global os.clock()
local a=0
while a<ITT_COUNT do
	a=a+1
end
print(global string.format("%i standard iterations took %.5f seconds",global ITT_COUNT,global os.clock()-local start))

local start=global os.clock()
local a=0
while local a<global ITT_COUNT do
	local a=local a+1 //do not optimize this to a++, that defeats the purpose of this test.
end
print(global string.format("%i local iterations took %.5f seconds",global ITT_COUNT,global os.clock()-local start))

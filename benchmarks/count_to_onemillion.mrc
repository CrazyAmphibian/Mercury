local start=global os.clock()
local a=0
while a<1000000 do
	a=a+1
end
print(global string.format("standard iteration took %.5f seconds",global os.clock()-local start))

local start=global os.clock()
local a=0
while local a<1000000 do
	local a=local a+1
end
print(global string.format("locals iteration took %.5f seconds",global os.clock()-local start))

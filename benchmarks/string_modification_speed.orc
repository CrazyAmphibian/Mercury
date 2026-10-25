global ITT_COUNT = 1000000

local str1="abcdefghijklmnopqrstuvwxyz"
local str2="1234567890"


local n=ITT_COUNT

local start=global os.clock()
while local n do
	local concstr=str1..str2
	local concstr..=local concstr
	local n--
end
print(global string.format("%i concatenation iterations took %.5f seconds",global ITT_COUNT,global os.clock()-local start))
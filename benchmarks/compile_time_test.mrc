local iteration_count=10000
local code="
if true then return 20 end
while 14 do break end
n=function(a,b) return b,a end
a=5 //comment
b=10
n(a,b)
goto point
if nil then print(1) elseif 6 then print(55) else print(7) end
point:
print(\"aaaaaa\")
"



local n=iteration_count
local start=os.clock()
while n do
	local f=compile(code)
	if type(f)!=TYPE_FUNCTION then
		print("code failed to successfully compile.")
		return 1
	end
	n--
end
print(string.format("%i code compile iterations took %f seconds",iteration_count,os.clock()-start) )





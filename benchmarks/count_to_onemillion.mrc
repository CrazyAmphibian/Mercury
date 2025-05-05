start=os.clock()
a=0
while a<1000000 do
	a=a+1
end
time=os.clock()-start
print("standard iteration took "..time.." seconds")

start=os.clock()
local a=0
while local a<1000000 do
	local a=local a+1
end
time=os.clock()-start
print("iteration using locals took "..time.." seconds")
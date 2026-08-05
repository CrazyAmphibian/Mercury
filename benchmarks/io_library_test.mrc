//the io library is a bit finnicky, so for tests, we'll just stick to universal functions that won't mess with the hard drive

local sd=[423,12341,1211,9999,1.0]
local data={1,-1,300,9999999,1.0,true,false,"","t","TEST!",sd}

local dmp=io.serialize(data)

if dmp==nil then
	print("io.serialize failed test, unable to serialize input data")
	return 1
end

local retrived=io.deserialize(dmp)

if retrived==nil then
	print("io.deserialize failed test, unable to recreate data")
	return 1
end

local n=0
while n<10 do //only go to 9 because the 10th element is an array
	if retrived[n]!=data[n] then
		print(string.format("io.deserialize failed test, element %i was incorrect. expected %s, got %s",n,data[n],retrived[n]))
		return 1
	end
	n++
end

local rsd=retrived[10]
if type(rsd)!=TYPE_ARRAY then
	print("io.deserialize failed test, element 10 was not an array")
	return 1
end
n=0
while n<#sd do
	if rsd[n]!=sd[n] then
		print(string.format("io.deserialize failed test, array element %i was incorrect. expected %s, got %s",n,sd[n],rsd[n]))
		return 1
	end
	n++
end

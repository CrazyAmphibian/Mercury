
func=function(n) a=n b=n end
env={"a"=5}
rcall(func,env,1)

if (env["a"]!=1) || (env["b"]!=1) then
	print("rcall did not set the correct variables while running")
	return 1
end


arrt=[0=1,1=7,6=11]
if dump(arrt)!="[0=1,1=7,6=11,]" then
	print("dump did not output the correct string")
	return 1
end 

local w=nil
f=compile("w=10")
if type(f)!=TYPE_FUNCTION then
	print("compile failed to compile code")
	return 1
else
	f()
	if w!=10 then
		print("compile did not run correctly")
		return 1
	end
end


if tostring(12)!="12" then
	print("tostring didn't make a string")
	return 1
end

if (tonumber("12.7")!=12.7) || (tonumber(false)!=0) then
	print("tonumber failed number generation")
	return 1
end

if (toint("12.7")!=12) || (toint(false)!=0) then
	print("toint failed number generation")
	return 1
end

if (tofloat("12")!=12.0) || (tofloat(false)!=0.0) then
	print("tofloat failed number generation")
	return 1
end

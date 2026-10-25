tfunc=function(n)
	local o=n%1234
	while n do n-- end
	return o
end

t1=thread.new(tfunc,nil,1000000)
t2=thread.new(tfunc,nil,900000)

thread.await(t1)
thread.await(t2)


local var=nil
if thread.checkerror(t1) then
	print("thread 1 encountered an error while running")
	return 1
elseif thread.getcount(t1) then
	var=thread.fetch(t1)
	if var!=460 then
		print("thread 1 returned an incorrect value "..var)
		return 1
	end
else
	print("thread 1 did not return a value")
	return 1
end
var=nil
if thread.checkerror(t2) then
	print("thread 2 encountered an error while running")
	return 1
elseif thread.getcount(t2) then
	var=thread.fetch(t2)
	if var!=414 then
		print("thread 2 returned an incorrect value "..var)
		return 1
	end
else
	print("thread 2 did not return a value")
	return 1
end

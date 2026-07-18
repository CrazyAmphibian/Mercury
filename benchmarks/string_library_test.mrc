str="racecar"
str2="hello, world!"
if string.reverse(str)!=str then
	print("string.reverse test 1 failed!")
	return 1
elseif string.reverse(str2)==str2 then
	print("string.reverse test 2 failed!")
	return 1
end

if string.sub(str,#str-3,#str-1) !="car" then
	print("string.sub test failed!")
	return 1
end

if string.find(str2,"o",5)!=8 then
	print("string.find test failed!")
	return 1
end

if string.replace(str2,"l","z",2)!="hezzo, world!" then
	print("string.replace test failed!")
	return 1
end

s=string.toarray(str)
iterate(s,function(i,v,a)
	a[i]=v+1
end)
if string.fromarray(s)!="sbdfdbs" then
	print("string.toarray and string.fromarray test failed!")
	return 1
end

if string.upper(str)!="RACECAR" then
	print("string.upper test failed!")
	return 1
elseif string.lower(string.upper(str2))!=str2 then
	print("string.upper and string.lower test failed!")
	return 1
end

if string.format("%s: %02i:%02i %.2f","time",12,4,#str)!="time: 12:04 7.00" then
	print("string.format test failed!")
	return 1
end

if string.pfind("result: 12.7","-%?%n%+.%?%n%*")!=8 then
	print("string.pfind test failed!")
	return 1
end

if string.pextract("result: 12.7","-%?%n%+.%?%n%*")[0]!="12.7" then
	print("string.pextract test failed!")
	return 1
end

if string.pcount("result: 12.7","-%?%n%+.%?%n%*")!=1 then
	print("string.pcount test failed!")
	return 1
end

if string.preplace("result: 12.7","-%?%n%+.%?%n%*","[number]")!="result: [number]" then
	print("string.preplace test failed!")
	return 1
end

if string.escape("test\\\"\"\\test")!="test\\\\\\\"\\\"\\\\test" then //yeah, this is a bit of a mess.
	print("string.escape test failed!")
	return 1
end

if string.escape_url("hello?thing/huh.4")!="hello%3Fthing%2Fhuh.4" then //yeah, this is a bit of a mess.
	print("string.escape_url test failed!")
	return 1
end

if string.escape_c("hello\nworld!\0")!="hello\\nworld!\\0" then //yeah, this is a bit of a mess.
	print("string.escape_c test failed!")
	return 1
end

if string.escape_html("<p>hello there</p>")!="&lt;p&gt;hello there&lt;/p&gt;" then //yeah, this is a bit of a mess.
	print("string.escape_c test failed!")
	return 1
end

print("string library test passed!")

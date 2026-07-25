a=[0=1,2=2,4=3,6=4]
b=array.copy(a)

if a==b then
	print("array.copy failed test, copied array is the same object.")
	return 1
else
	passed=true
	iterate(a,function(key,val,obj)
		if b[key]!=val then passed=false return 1 end
	end)
	if !!passed then
		print("array.copy failed test, copied array has differing data.")
		return 1
	end
end

array.flush(a)
passed=false
iterate(a,function(key,val,obj)
	if b[key]!=val then passed=true end
end)
if !!passed then
	print("array.flush failed test, flushed array has same data.")
	return 1
end

if (a[0]!=b[0]) || (a[1]!=b[2]) || (a[2]!=b[4]) || (a[3]!=b[6]) then
	print("array.flush failed test, incorrect data detected.")
	return 1
end


array.insert(a,5,2)
if (a[2]!=5) || (a[3]!=3) || (a[4]!=4) then
	print("array.insert failed test, incorrect data detected.")
	return 1
end

array.remove(a,2)
if (a[2]!=3) || (a[3]!=4) then
	print("array.remove failed test, incorrect data detected.")
	return 1
end

array.swap(a,0,3)
if (a[0]!=4) || (a[3]!=1) then
	print("array.swap failed test, incorrect data detected.")
	return 1
end

if array.concat(a,",")!="4,2,3,1" then
	print("array.concat failed test. expected \"4,2,3,1\", got "..array.concat(a,",") )
	return 1
end

array.sort(a,array.SORTING_LESSER_TO_GREATER)
if (a[0]!=1) || (a[1]!=2) || (a[2]!=3) || (a[3]!=4) then
	print("array.sort SORTING_LESSER_TO_GREATER test failed!")
	return 1
end
array.sort(a,array.SORTING_GREATER_TO_LESSER)
if (a[0]!=4) || (a[1]!=3) || (a[2]!=2) || (a[3]!=1) then
	print("array.sort SORTING_GREATER_TO_LESSER test failed!")
	return 1
end

a[0]*=-1
a[3]*=-1

array.sort(a,array.SORTING_LESSER_TO_GREATER_MAGNITUDE)
if (a[0]!=-1) || (a[1]!=2) || (a[2]!=3) || (a[3]!=-4) then
	print("array.sort SORTING_LESSER_TO_GREATER_MAGNITUDE test failed!")
	return 1
end
array.sort(a,array.SORTING_GREATER_TO_LESSER_MAGNITUDE)
if (a[0]!=-4) || (a[1]!=3) || (a[2]!=2) || (a[3]!=-1) then
	print("array.sort SORTING_GREATER_TO_LESSER_MAGNITUDE test failed!")
	return 1
end

c=["a","b","ab","ba","bb","c"]
array.sort(c,array.SORTING_ALPHABETICAL_A_TO_Z)
if (c[0]!="a") || (c[1]!="ab") || (c[2]!="b") || (c[3]!="ba") || (c[4]!="bb") || (c[5]!="c") then
	print("array.sort SORTING_ALPHABETICAL_A_TO_Z test failed!")
	return 1
end

array.sort(c,array.SORTING_ALPHABETICAL_Z_TO_A)
if (c[0]!="c") || (c[1]!="bb") || (c[2]!="ba") || (c[3]!="b") || (c[4]!="ab") || (c[5]!="a") then
	print("array.sort SORTING_ALPHABETICAL_Z_TO_A test failed!")
	return 1
end

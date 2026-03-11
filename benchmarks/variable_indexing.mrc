/*
purpose: to verify that indexing variables (get and set) works correctly
*/

a={"a"=1,"b"=2,"c"=3}
a.d=4
a["e"]=5
b=[1,2,3]
b[3]=4


sa=a.a+a.b+a.c+a.d+a.e
sb=a["a"]+a["b"]+a["c"]+a["d"]+a["e"]
sc=b[0]+b[1]+b[2]+b[3]

if sa!=sb then
	print("implicit and explicit table indexing failed parity!")
else
	if sc!=10 then
		print("array indexing failed!")
	elseif sa!=15 then
		print("table indexing failed!")
	else
		print("indexing test passed!")
	end
end

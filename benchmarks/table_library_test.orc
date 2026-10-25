
t={"a"=1,"b"=2,"c"=3}
t2=table.copy(t)

if t==t2 then
	print("table.copy failed test, copied table is the same object.")
	return 1
else
	passed=true
	iterate(t,function(key,val,obj)
		if t2[key]!=val then passed=false return 1 end
	end)
	if !!passed then
		print("table.copy failed test, copied table has differing data.")
		return 1
	end
end


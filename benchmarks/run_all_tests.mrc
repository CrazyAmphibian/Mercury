iterate(io.getfiles(""),function(i,v,a)
	if v=="run_all_tests.mrc" then return end
	io.post(string.format("==========%s==========\n",v))
	local f=io.open(v,"r")
	local c=io.read(f)
	local r=compile(c)
	if type(r)==TYPE_FUNCTION then r() else print(r) end
	io.close(f)
end)
io.post("====================\npress any key to end...")
io.input()
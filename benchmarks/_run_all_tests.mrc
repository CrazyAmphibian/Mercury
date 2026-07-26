local total=0
local passes=0
iterate(io.getfiles(""),function(i,v,a)
	if v=="_run_all_tests.mrc" then return end
	if !!string.find(v,".mrc") then return end
	//print(v)
	total++
	local f=io.open(v,"r")
	local c=io.read(f)
	local r=compile(c)
	if type(r)==TYPE_FUNCTION then
		local fr=r()
		if fr then
			io.post(string.format("script %s encountered an error while running and returned %s\n",v,fr) )
		else
			passes++
		end
	else print("error while compiling "..v..": "..r) end
	io.close(f)
end)
io.post(string.format("\n%i/%i tests passed.\npress any key to exit...",passes,total))
io.input()

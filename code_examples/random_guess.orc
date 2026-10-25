math.randomseed(os.time())
while true do
	local number=math.randomint(1,10)
	io.post("guess what number i am thinking of! ")
	local num=tonumber(io.prompt())
	
	if num==nil then
		io.post("that's not even a number!\n")
	else
		if num==number then
			io.post("that's right!\n")
		else
			io.post("not quite! i was thinking of "..number..".\n")
		end
	end

end

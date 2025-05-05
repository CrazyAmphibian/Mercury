io.post("please enter a number:")
n1=tonumber(io.prompt())
if n1==nil then io.post("sorry, that is not a number.") return end
io.post("please enter a second number:")
n2=tonumber(io.prompt())
if n2==nil then io.post("sorry, that is not a number.") return end

io.post("the sum of those numbers is:".. (n1+n2) .. "\n")

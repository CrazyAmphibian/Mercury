function factorial(n)
local sum=0
	while n do
		sum=sum+n
		n=n-1
	end
	return sum
end


print(factorial(8))
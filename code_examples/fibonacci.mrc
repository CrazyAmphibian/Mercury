function fibonacci(n)
	local total=0
	local num1=1
	local num2=0
	while n do
		total=num1+num2
		num2=num1
		num1=total //test
		n=n-1
	end
	return total
end

print(fibonacci(10))
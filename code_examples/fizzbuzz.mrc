n=1
while n<=100 do
	if n%3 then
		if n%5 then
			print("FizzBuzz")
		else
			print("Fizz")
		end
	else
		if n%5 then
			print("Buzz")
		else
			print(n)
		end
	end
	n=n+1
end

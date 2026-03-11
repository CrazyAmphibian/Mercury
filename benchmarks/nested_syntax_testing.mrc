/*
purpose: to test that the compiler correctly generates code for deeply nested code and whatnot
*/
count_1=0
count_2=0

testfunc=function(n)
	return n==5
end

start:

while count_1<123 do
	if testfunc(count_1) then
		if count_2==1 then
			count_1=44
			break
		elseif count_2==0 then
			count_1=124
			continue
		end
	end
	if (count_1%10)==6 then
		count_1~=0x7
		count_1&=0xff
		continue
	end
	middle:
	count_1++
end


count_2++
if count_2==1 then
	goto start
elseif count_2==2 then
	goto middle
end


if(count_1==125) && (count_2 ==3) then
	print("syntax test passed!")
else
	print(string.format("syntax test failed! count_1 expected 125, was %i; count_2 expected 3, was %i",count_1,count_2))
end

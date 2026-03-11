/*
purpose: to test that operations work and take args in the right order
*/

a=4
a+=1
a%=3
a*=7
a-=3
a^=2
a/=3
a=!a
a\=2
a|=2
a=-a
a~=181983
a&=255

if a!=137 then
	print("operations test failed! we got "..a .." but expected 137.")
else
	print("operations test passed!")
end

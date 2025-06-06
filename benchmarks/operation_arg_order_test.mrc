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
a\=2
a|=2
a~=181983
a&=255

if a!=201 then
	print("operations test failed!")
else
	print("operations test passed!")
end

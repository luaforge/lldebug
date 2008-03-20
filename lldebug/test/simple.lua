--
-- simple test
--

for i = 0, 100 do
	print("i = " .. i)
end

local text = "ab abcd ab";
for w in string.gmatch(text, "a.*b") do
	print("ret = " .. w .. "\n"); 
end

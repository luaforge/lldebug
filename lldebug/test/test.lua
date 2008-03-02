--
-- テストプログラム (デソ 日本語表示)
--

debug = require "debug"
table = require "table"
string = require "string"

require "strict"

-- recursive
local function self_recursive(count, through)
	through = through or {}
	
	if count <= 0 then
		print("self_recursive was done: " .. tostring(through))
		return
	end
	
	through[count] = "recorded"
	return self_recursive(count - 1, through)
end

-- Environment table
local function env_func()
	local function f(t, i)
		return os.getenv(i)
	end
	setmetatable(getfenv(), {__index=f})

	-- an example
	print(a, USER, PATH)
end

-- Coroutine function
local function co_func_creator(count)
	local function co_func_()
		while true do
			print("coroutine func: " .. count)
			count = count + 1

			coroutine.yield()
		end
	end
	
	return coroutine.create(co_func_)
end

local co = co_func_creator(100)
local function co_func()
	coroutine.resume(co)
end

-- Dump 'obj' to string.
local function dump_func(obj)
	return string.dump(obj)
end

local tab = {
	[0] = "テスト",
	deep = {x = nil}
}
tab.self = tab

for i = 0, 100 do
	local str = [[テストデソ]]
	print(i, str)

	self_recursive(100)
	dump_func(function() return 100 end)
	env_func()
	co_func()
end

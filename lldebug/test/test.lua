--
-- 日本語表示
--

debug = require "debug"

function t()
  if 0 == 0 then
    --print(debug.traceback("msg"))
    callstack()
    return
  end

  t(i - 1)
end

function g(i)
  while true do
    print(i)
    t()
    i = coroutine.yield()
  end
end

local co
local function f(i)
  if co == nil then
    co = coroutine.create(g)
  end

  function inner()
    local test = {x = function() end}
	  --print(test)
  end

  local tab = {[0] = "テスター", deeptab = {["x"]=coroutine.create(f)}}
  tab.tt = tab
  coroutine.resume(co, i)
  inner()
end

local tab = {[0] = "テスト", deep = {x=coroutine.create(f)}}
tab.tt = tab
local i = 0
local str = [[テストデソ]]
while 1 do
  print(i, str)
  f(i)
  i = i + 1
end

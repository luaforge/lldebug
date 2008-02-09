--
-- 日本語表示
--

--require "E:\\programs\\develop\\lldebug\\test\\strict"
--require "strict"

debug = require "debug"
require "table"

function t()
  if 0 == 0 then
    print(debug.traceback("msg"))
    return
  end

  t(i - 1)
end

function f1(e)
  setfenv(1,e)
  print(a)
end
function f2(f)
  a=123
  local e1 = getfenv(0)
  e1 = getfenv(1)
  e1 = getfenv(2)
  f(getfenv(0))
end
f2(f1) 

function g(i)
  while true do
    print(i)
    t()
    i = coroutine.yield()
  end
end

local co = nil
local function f(i)
  if co == nil then
    co = coroutine.create(g)
  end

  local function inner()
    local test = {x = function() end}
	  --print(test)
  end

  local tab = {[0] = "テスター", deeptab = {["x"]=coroutine.create(f)}}
  tab.tt = tab
  coroutine.resume(co, i)
  inner()
end

tab = {[0] = "テスト", deep = {x=coroutine.create(f)}}
tab.tt = tab
local i = 0
local str = [[テストデソ]]
while 1 do
  print(i, str)
  f(i)
  print(x)
  i = i + 1
end

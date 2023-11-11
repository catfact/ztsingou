local e = include('lib/ztsingou-engine')

local engine_init_callback = function()
    params:bang()
end

init = function()
    e.init(engine_init_callback)
end

cleanup = function()
    e.cleanup()
end

key = function(n, z) end

enc = function(n, z) end

redraw = function() end

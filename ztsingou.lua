local e = include('ignore/ztsingou-engine')

local engine_init_callback = function()
    
end

init = function()
    e.init(engine_init_callback)
end
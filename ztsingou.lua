local e = include('ignore/ztsingou-engine')

local engine_init_callback = function()
    for i,k in ipairs(e.param_ids) do
        local min = e.param_ranges[k][1]
        local max = e.param_ranges[k][2]
        params:add_control(k, k, controlspec.new(min, max, 'lin', 0, min, ''), 
            function(v) 
                e.set_param(idx, v) 
            end
        )
    end
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

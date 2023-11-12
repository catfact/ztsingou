local e = include('lib/ztsingou-engine')

local engine_init_callback = function()
    params:default()
    params:bang()
end

init = function()
    e.init(engine_init_callback)
end

cleanup = function()
    e.cleanup()
end

-- viewmodel
local vm = 
{
    l = False,
    r = False,
}

key = function(n, z) 
    --if z == 0 then return end
    if n == 1 then return end
    local x = math.random(1, 100) * 0.005
    if n == 2 then 
        if z == 0 then 
            vm.l = False
        else
            params:set('pluck_1', x)
            vm.l = True
        end
        redraw()
    elseif n == 3 then
        if z == 0 then 
            vm.r = False
        else
            params:set('pluck_2', x)
            vm.r = True
        end
        redraw()
    end
end

enc = function(n, z)
    if n == 1 then return end
    if n == 2 then
        params:delta('epsilon_1', z * 0.05)
    elseif n == 3 then
        params:delta('epsilon_2', z * 0.05)
    end
end

redraw = function()
    screen.clear()
    screen.level(15)
    if vm.l then
        screen.rect(0, 0, 64, 64)
        screen.fill()
    end
    if vm.r then
        screen.rect(64, 0, 64, 64)
        screen.fill()
    end
    screen.update()
end

local e = include('lib/ztsingou-engine')

local redraw_timer
local needs_redraw = false

local engine_init_callback = function()
    params:default()
    params:bang()
    redraw_timer = metro.init()
    redraw_timer.event = function()
        if needs_redraw then
            redraw()
            needs_redraw = false
        end
    end
    redraw_timer:start(1/15)
end

init = function()
    e.init(engine_init_callback)
end

cleanup = function()
    e.cleanup()
end

-- "constant" table of parameter assignment pairs
local ctl_assign = {}
for i,k in ipairs(e.param_string_keys) do    
    ctl_assign[i] = {k.."_1", k.."_2"}
end

-- controller state
local ctl = 
{
    alt = false,
    assign = 5    
}

-- viewmodel
--- FIXME: should update VM from param actions
--- (our quick-and-dirty param factory isn't flexible enough)
local vm = 
{
    l = {
        pluck = false,
        label = 'beta_1',
        value = '?'
    },
    r = {
        pluck = false,
        label = 'beta_1',
        value = '?'
    },
}

key = function(n, z) 
    if n == 1 then 
        if z > 0 then
            ctl.alt = true
        else
            ctl.alt = false
        end
    else 
        if n == 2 then 
            if z == 0 then 
                vm.l.pluck = false
            else
                -- FIXME: really just want to bang the param
                -- (using zero delta doesn't seem to work?)
                params:delta('pluck_1', 0.0001)
                vm.l.pluck = true
            end
        elseif n == 3 then
            if z == 0 then 
                vm.r.pluck = false
            else
                params:delta('pluck_2', 0.0001)
                vm.r.pluck = true
            end
        end
    end
    needs_redraw = true
end

enc = function(n, z)
    if n == 1 then 
        local assign = ctl.assign + z
        if assign < 1 then assign = 1 end
        if assign > #ctl_assign then assign = #ctl_assign end
        ctl.assign = assign
        vm.l.label = ctl_assign[ctl.assign][1]
        vm.r.label = ctl_assign[ctl.assign][2]
    end
    if n == 2 then
        -- if ctl.alt then
        --     params:delta('epsilon_1', z * 0.2)
        --     vm.l.value = params:get('epsilon_1')
        -- else
        --     params:delta('beta_1', z * 0.05)
        --     vm.l.value = params:get('beta_1')
        -- end

        -- FIXME: would be nice to vary delta by assigned param
        local k = ctl_assign[ctl.assign][1]
        local d = ctl.alt and 0.02 or 0.1
        params:delta(k, z * d)
        vm.l.value = params:get(k)
    elseif n == 3 then
        -- if ctl.alt then
        --     params:delta('epsilon_2', z * 0.2)
        --     vm.r.value = params:get('epsilon_2')
        -- else
        --     params:delta('beta_2', z * 0.05)
        --     vm.r.value = params:get('beta_2')
        -- end
        local k = ctl_assign[ctl.assign][2]
        params:delta(k, z * 0.05)
        vm.r.value = params:get(k)
    end
    needs_redraw = true
end

function redraw()
    screen.clear()
    screen.level(15)
    if vm.l.pluck then
        screen.rect(10, 10, 44, 44)
        screen.stroke()
    end
    if vm.l.label then
        screen.move(12, 20)
        screen.text(vm.l.label)
    end
    if vm.l.value then
        screen.move(12, 30)
        screen.text(vm.l.value)
    end
    if vm.r.pluck then
        screen.rect(74, 10, 44, 44)
        screen.stroke()
    end
    if vm.r.label then
        screen.move(76, 20)
        screen.text(vm.r.label)
    end
    if vm.r.value then
        screen.move(76, 30)
        screen.text(vm.r.value)
    end
    screen.update()
end

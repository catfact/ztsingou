-- the engine accepts zero-based integer parameter indexes, not string keys
-- this is quite a bit more efficient, at the cost of some bookkeeping here

-- first, an array of readable parameter names, in specific order
local param_string_keys = { 
	amp, 
	pickupPos1, pickupPos2, excitePos, 
	beta, epsilon, rho, 
	pluck
}

-- table associating parameter names to zero-based indices...
-- this matches enum in engine code
-- order need not match param_string_keys
local param_string_ids = {
	amp = 0,
	pickupPos1 = 1,
	pickupPos2 = 2,
	excitePos = 3,
	beta = 4,
	epsilon = 5,
	rho = 6,
	pluck = 7
}

-- ...and ranges 
local param_string_ranges = { 
	amp = {0, 1},
	pickupPos1 = {1, 14},
	pickupPos2 = {1, 14},
	excitePos = {1, 14},
	beta = {0, 30},
	epsilon = {0, 20},
	rho = {0, 20},
	pluck = {0, 1}
}

--- ... do it all again for global params
local param_global_keys = { 
	spread, mono, gain,
	ips, masses
}

local param_global_ids = {
	spread = 0,
	mono = 1,
	gain = 2,
	ips = 3,
	masses = 4
}

local param_global_ranges = { 
	spread = {0, 1},
	mono = {0, 1},
	gain = {0, 2},
	ips = {1, 32},
	masses = {3, 16}
}

--- engine-like table
local e = {}

-- create accessor functions for each parameter
-- FIXME: all parameters use the same OSC method, taking a string argument
--        this despite the fact that some params apply to all strings
--        (should have e.g. `/param` and `/param/string`)
for idx, id in ipairs(param_string_ids) do
    e[id] = function (string, value)
		-- convert the string index to zero-base here
        osc.send(client, "/param/string", {string-1, idx, value})
    end
end

for id, idx in ipairs(param_global_ids) do
    e[id] = function (value)
        osc.send(client, "/param/global", {idx, value})
    end
end

local did_init = false

local add_params = function()
	print("adding engine params...")
	local params = include("lib/params")
	params:add_separator("ztsingou")
	params:add_group("ztsingou")

	-- TODO: building these programatically is concise, but quick and dirty
	-- can/should fine tune behaviors per param
	-- - `ips`, `masses` should be integers
	-- - `pluck` should not be saveable
	-- - `beta`, `epsilon`, `rho` should be scaled more deliberately / abstracted

	for i,k in ipairs(param_string_ids) do
        local min = param_string_ranges[k][1]
        local max = param_string_ranges[k][2]
		for string=1,2 do
			local local id= k.."_"..string
			params:add_control(k, k, controlspec.new(min, max, 'lin', 0, min, ''), 
				function(v) 
					e.set_param_string(string, idx, v) 
				end
			)
		end
    end
	for i,k in ipairs(param_global_ids) do
        local min = param_global_ranges[k][1]
        local max = param_global_ranges[k][2]
		params:add_control(k, k, controlspec.new(min, max, 'lin', 0, min, ''), 
			function(v) 
				e.set_param_global(idx, v) 
			end
		)
    end
end

--- initialize the engine by launching its process
--- @param callback function to be executed on receiving engine-ready OSC 
e.init = function(callback)
	--- FIXME: defining the global OSC handler here is not very tidy or robust
	--- in any case, we need to set up the handler before launching the engine process
	print("--- zt-engine init")

	print("--- setting OSC event handler")
	osc.event = function(path, args, from)
		print("osc event: "..path)
		if path == "/ztsingou/ready" and not did_init then
			print("engine ready!")
			did_init = true
			add_params()
			callback()
		end
	end

	print("--- running engine process...")
	local runsh = "/home/we/dust/code/ztsingou/lib/run-norns.sh"

	--- hm, norns.system_cmd() seems to get stuck here... 
	--- (works, but subsequent calls to system_cmd hang)
	-- norns.system_cmd(runsh)

	--- use os.execute() instead
	os.execute(runsh)
    
end

-- clean up the engine by sending a quit message
e.cleanup = function()
	osc.event = nil
	osc.send(client, "/quit")
	-- TODO: should verify that the engine process always exits
	-- or just (wait) and:
	-- os.execute("pidof ztsingou | xargs kill")
end

return e
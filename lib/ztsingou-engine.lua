-- the engine accepts zero-based integer parameter indexes, not string keys
-- this is quite a bit more efficient, at the cost of some bookkeeping here

-- first, an array of readable parameter names, in specific order
local param_keys = { 
	amp, spread, mono, gain, 
	pickupPos1, pickupPos2, excitePos, 
	beta, epsilon, rho, 
	pluck
}

-- tables associating parameter names to zero-based indices...
local param_ids = {}
for i,k in ipairs(param_keys) do
	param_id[k] = i-1
end

-- ...and ranges 
local param_ranges = { 
	amp = {0, 1},
	spread = {0, 1},
	mono = {0, 1},
	gain = {0, 2},
	pickupPos1 = {1, 14},
	pickupPos2 = {1, 14},
	excitePos = {1, 14},
	beta = {0, 30},
	epsilon = {0, 20},
	rho = {0, 20},
	pluck = {0, 1}
}

local e = {}
e.param_keys = param_keys
e.param_ids = param_ids
e.param_ranges = param_ranges

-- create accessor functions for each parameter
-- FIXME: all parameters use the same OSC method, taking a string argument
--        this despite the fact that some params apply to all strings
--        (should have e.g. `/param` and `/param/string`)
for id, idx in ipairs(param_ids) do
    e[id] = function (string, value)
		-- convert the string index to zero-base here
        osc.send(client, "/param", {string-1, idx, value})
    end
end

local did_init = false

--- initialize the engine by launching its process
--- @param callback function to be executed on receiving engine-ready OSC 
e.init = function(callback)
	norns.system_cmd("/home/we/dust/code/ztsingou/lib/run-norns.sh")

    --- FIXME: defining the global OSC handler here is not very tidy or robust
	osc.event = function(path, args, from)
		if path == "/ready" and not did_init then
			did_init = true
			callback()
		end
	end

	end
end

-- clean up the engine by sending a quit message
e.cleanup = function()
	osc.event = nil
	osc.send(client, "/quit")
	-- TODO: for test purposes, should wait here and then verify that the engine process has exited
end

return e
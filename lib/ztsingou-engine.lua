-- the engine accepts zero-based integer parameter indexes, not string keys
-- this is quite a bit more efficient, at the cost of some bookkeeping here

-- first, an array of readable parameter names, in specific order
-- these parameters have a separate value for each physically-modeled string
-- so setters for them take a string index and a value
local param_string_keys = { 

	-- output amplitude
	'amp', 

	-- pickup and excitation positions (2-15)
	'pickupPos1', 'pickupPos2', 'excitePos', 

	-- tension coefficient
	'beta',
	
	-- nonlinear stiffness coefficient
	'epsilon', 
	
	-- damping
	'rho', 

	-- stateless parameter; sets the position of the mass at the excitation point
	'pluck'
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

-- ...and ranges; each is {min, max, default}
local param_string_ranges = { 
	amp = {0, 1, 0.5},
	pickupPos1 = {1, 14, 2},
	pickupPos2 = {1, 14, 6},
	excitePos = {1, 14, 5},
	beta = {0, 30, 1},
	epsilon = {0, 20, 0},
	rho = {0, 20, 0.2},
	pluck = {0, 1, 0}
}

--- ... do it all again for "global" parameters
--- these have a single value for the whole instrument
local param_global_keys = { 
	-- amount of stereo spread between each pickup point on each string
	-- first pickup on each string is always panned hard left/right
	'spread', 

	-- increase to "mono-ize" the final output mix
	'mono', 

	-- input gain for excitation
	-- applies to both channels; each input channel goes to a different string
	'gain',

	-- iterations per sample. this is rounded to an integer. 
	-- increaseing IPS divides pitch, multiplies CPU
	'ips', 

	-- number of masses to compute per string
	-- changes the distribution of resonant modes,
	-- increases / decreases CPU load
	'masses'
}

local param_global_ids = {
	spread = 0,
	mono = 1,
	gain = 2,
	ips = 3,
	masses = 4
}

local param_global_ranges = { 
	spread = {0, 1, 1},
	mono = {0, 1, 0},
	gain = {0, 2, 0},
	ips = {1, 32, 16},
	masses = {3, 16, 8}
}

--- engine-like table
local e = {}
local client = { "127.0.0.1", "9998" }

-- create accessor functions for each parameter
for id, idx in pairs(param_string_ids) do
    e[id] = function (string, value)
		-- convert the string index to zero-base here
		
		print("setting param: "..id.."["..string.."]")
        osc.send(client, "/param/string", {string-1, idx, value})
    end
end

for id, idx in pairs(param_global_ids) do
    e[id] = function (value)
		print("setting param: "..id)	
        osc.send(client, "/param/global", {idx, value})
    end
end

--tab.print(e)
print("--- engine table: ")
for k,v in pairs(e) do
	print(""..k.."\t"..tostring(v))
end

local did_init = false

local add_params = function()
	print("adding engine params...")
	params:add_separator("ztsingou")

	-- TODO: building these programatically is concise, but quick and dirty
	-- can/should fine tune behaviors per param
	-- - `ips`, `masses` should be integers
	-- - `pluck` should not be saveable
	-- - `beta`, `epsilon`, `rho` should be scaled more deliberately / abstracted

--	tab.print(param_string_keys)

	for _,k in ipairs(param_string_keys) do
		local idx = param_string_ids[k]
        local min = param_string_ranges[k][1]
        local max = param_string_ranges[k][2]
        local init = param_string_ranges[k][3]
		for string=1,2 do
			local id= k.."_"..string
			print("--- adding param: "..id)
			params:add_control(id, id, controlspec.new(min, max, 'lin', 0, init, ''))
			params:set_action(id, function(v) 
				(e[k])(string, v)
			end)	
		end
    end
	for _,k in ipairs(param_global_keys) do
		local idx = param_global_ids[k]
        local min = param_global_ranges[k][1]
        local max = param_global_ranges[k][2]
        local init = param_global_ranges[k][3]
		print("--- adding param: "..k)
		params:add_control(k, k, controlspec.new(min, max, 'lin', 0, init, ''))
		params:set_action(k, function(v) 
			(e[k])(v)
		end)
    end
end

--- initialize the engine by launching its process
--- @param callback function to be executed on receiving engine-ready OSC 
e.init = function(callback)
	print("--- zt-engine init")

	-- add an OSC event handler
	-- to keep this module a little more portable, 
	-- we'll monkey-patch the script's handler instead of just overwriting it
	-- (wouldn't hurt to have a more expressive responder system in norns)
	print("--- setting OSC event handler")
	local ev = osc.event
	osc.event = function(path, args, from)
		if ev~=nil then ev(path, args, from) end
		-- print(" --- ztsingou-engine osc event: "..path)
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

	--- use os.execute() instead, for now
	os.execute(runsh)
    
end

-- clean up the engine by sending a quit message
e.cleanup = function()
	-- NB: no need to clean up up the global OSC handler, norns does it

	osc.send(client, "/quit")
	-- TODO: should verify that the engine process always exits
	-- or just (wait) and:
	-- os.execute("pidof ztsingou | xargs kill")
end

return e
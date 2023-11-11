local param_ids = {
    	amp = 0,
    	spread = 1,
    	mono = 2,
    	gain = 3,
    	pickupPos1 = 4,
    	pickupPos2 = 5,
    	excitePos = 6,
    	beta = 7,
    	epsilon = 8,
    	rho = 9,
    	pluck = 10
}

local e = {}

for id, idx in ipairs(param_ids) do
    e[id] = function (string, value)
        osc.send(client, "/param", {string, idx, value})
    end
end
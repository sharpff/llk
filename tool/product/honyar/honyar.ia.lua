--[[
	rule template
	please fill the template during CUSTOMIZATION START/END
]]

--[[
	EXPORT
	as the prefix of the rule name
	E.g. 10000100091000610006ABCDEF.lua.bin
	lua5.3.0_win_slim.exe dooya/dooya.ia.lua s2GetRuleInfo
]]
function s2GetRuleInfo()
	-- TEMPLATE START
	nodeKind = '10000100091000610006'
	-- TEMPLATE END
	return string.len(nodeKind), nodeKind
end

--[[
	EXPORT
	if the rule has been expired
	statusForReserved: json (status + utc)

	E.g. utc:1456129920179 (2016/2/22 16:32:0)      h: (339)0x153 l: 136006835(0x081b4cb3)
	lua5.3.0_win_slim.exe dooya/dooya.ia.lua s2IsValid {\"status\":{},\"utcH\":339,\"utcL\":136006835}
]]
function s2IsValid(statusForReserved)
	local valid = 0
	-- print (statusForReserved)
	local tb = cjson.decode(statusForReserved)
	local utcL = tb["utcL"]
	local utcH = tb["utcH"]

	-- CUSTOMIZATION START
	local baseUtcL = 0x081b4cb3
	local baseUtcH = 0x153
	if utcH >= baseUtcH and utcL > baseUtcL then
		valid = 1
	end
	-- CUSTOMIZATION END
	return valid
end

-- function s2IsValidExt(statusForReserved, utc)
-- 	print (utc)
-- end

--[[
	EXPORT
	return value: [repeating call or status trig, AND or OR]
	lua5.3.0_win_slim.exe dooya/dooya.ia.lua s2GetRuleType
]]
function s2GetRuleType()
	local repeatOrTrig = 0
	local isAnd = 0
	-- repeating call or status trig, AND or OR
	-- CUSTOMIZATION START
	repeatOrTrig = 0
	isAnd = 0
	-- CUSTOMIZATION END
	return 0, 0
end

--[[
	EXPORT
	the being reserved node info(uuid)
	lua5.3.0_win_slim.exe dooya/dooya.ia.lua s2GetBeingReservedInfo
]]
function s2GetBeingReservedInfo()
	local reserved = ''
	a, b = s2GetRuleInfo()
	-- CUSTOMIZATION START
	macAddr = "FFFFFFFFFFFF"
	-- CUSTOMIZATION END
	reserved = b .. macAddr
	return string.len(reserved), reserved
end

--[[
	EXPORT
	status check
	statusForReserved: json (status + utc)
	return value: none 0 for ok, otherwise opposite
	E.g.
	lua5.3.0_win_slim.exe dooya/dooya.ia.lua s2IsConditionOK {\"status\":{\"action\":1},\"utcH\":339,\"utcL\":136006835}
]]
function s2IsConditionOK(statusForReserved)
	local ok = 0
	-- to decode the json, and do judgement
	local tb = cjson.decode(statusForReserved)
	local status = tb["status"]

	-- CUSTOMIZATION START
	if (status["action"] == 1) then
		ok = 1
	end
	-- CUSTOMIZATION END

	return ok
end

--[[
	EXPORT
	the self control cmd(json)
	E.g.
	lua5.3.0_win_slim.exe dooya/dooya.ia.lua s2GetSelfCtrlCmd
]]
function s2GetSelfCtrlCmd()
	--[[
	to get the cmd for self ctrl
	]]
	local selfCtrl = ''
	
	-- CUSTOMIZATION START
	selfCtrl = '{"action":1}'
	-- CUSTOMIZATION END
	return string.len(selfCtrl), selfCtrl
end

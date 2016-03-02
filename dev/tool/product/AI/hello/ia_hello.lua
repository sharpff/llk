--[[ 
	THIS IS IA SCRIPT
  ]]
--[[
	rule template
	please fill the template during CUSTOMIZATION START/END
]]
function cmpUTC(utcH, utcL, utcBaseH, utcBaseL) 
	local ret = -1
	if (utcH >= utcBaseH) and (utcL > utcBaseL) then
		-- print ("abc\r\n")
		ret = 1
	elseif (utcH == utcBaseH) and (utcL == utcBaseL) then
		-- print ("cba\r\n")
		ret = 0
	end
	-- print ("aaa\r\n")

	return ret
end



--[[
	EXPORT
	if the rule has been expired
	statusForReserved: json (status + utc)

	E.g. utc:1456129920 (2016/2/22 16:32:00(8+))      h: (339)0x153 l: 136006835(0x081b4cb3)
	lua5.3.0_win_slim.exe dqca9531/dqca9531.ia.lua s2IsValid {\"status\":{},\"utcH\":339,\"utcL\":136006835}
]]
function s2IsValid(statusForReserved)
	local valid = 0
	-- print (statusForReserved)
	local tb = cjson.decode(statusForReserved)
	local utcL = tb["utcL"]
	local utcH = tb["utcH"]

	-- CUSTOMIZATION START
	local utcBaseH = 0x081b4cb3
	local utcBaseL = 0x153
	-- if (cmpUTC(utcH, utcL, utcBaseH, utcBaseL) == 1) then 
		valid = 1
	-- end
	-- CUSTOMIZATION END
	return valid
end

--[[
	EXPORT
	return value: [repeating call or status trig, AND or OR]
	lua5.3.0_win_slim.exe dqca9531/dqca9531.ia.lua s2GetRuleType
]]
function s2GetRuleType()
	local repeatOrTrig = 0
	local isAND = 0
	-- repeating call or status trig, AND or OR
	-- CUSTOMIZATION START
	repeatOrTrig = 0
	isAND = 0
	-- CUSTOMIZATION END
	return 0, 0
end

--[[
	EXPORT
	the being reserved node info(uuid(s))
	lua5.3.0_win_slim.exe qca9531/qca9531.ia.lua s2GetBeingReservedInfo
]]
function s2GetBeingReservedInfo()
	local reserved = ''
	local count = 0
	-- local tblInfo = {}
	-- CUSTOMIZATION START
	local tblInfo = {'10000100011000510005FFFFFFFFFFFF', '20000100011000510005FFFFFFFFFFFF'}
	reserved = tblInfo[1] .. tblInfo[2]
	-- CUSTOMIZATION END
	print(string.format("[LUA] tblInfo[%d]\r\n", #tblInfo))
	return #tblInfo, string.len(reserved), reserved
end

--[[
	EXPORT
	status check
	statusForReserved: json (status + utc)
	statusPrivious: json (status + utc)
	return value: none 0 for ok, otherwise opposite
	E.g.
	lua5.3.0_win_slim.exe qca9531/qca9531.ia.lua s2IsConditionOK {\"status\":{\"action\":1},\"utcH\":339,\"utcL\":136006835}
]]
function s2IsConditionOK(statusForReserved, statusPrivious)
	local ok = 0
	local toStoreStatus = 0
	-- to decode the json, and do judgement
	local tb = {}
	local tbOld = {}
	-- print(string.format("[LUA] statusForReserved [%d][%s] \r\n", string.len(statusForReserved), statusForReserved))
	-- print(string.format("[LUA] statusPrivious [%d][%s] \r\n", string.len(statusPrivious), statusPrivious))
	if not (nil == statusForReserved) then
		tb = cjson.decode(statusForReserved)
	end
	if not (nil == statusPrivious) then
		tbOld = cjson.decode(statusPrivious)
	end
	local status = tb["status"]
	local statusOld = tbOld["status"]
	local uuid = tb["uuid"]

	-- CUSTOMIZATION START
	-- 1. 
	-- for status switched
	toStoreStatus = 1
	if (not (nil == status) and (status["pwr"] == 1)) and
		((nil == statusOld) or not (statusOld["pwr"] == 1)) then
		ok = 1
	end
	
	-- 2. 
	-- for single AND
	-- toStoreStatus = 1
	-- if ((status["pwr"] == 2) and (status["action"] == 1) and 
	-- 	((nil == statusOld) or ((not statusOld["pwr"] == 2) or (not statusOld["action"] == 1))))
		-- ok = 1
	-- end

	-- 3. 
	-- for time + status (in 5 secs)
	-- toStoreStatus = 1
	-- if (status["pwr"] == 3) then
	-- 	local utcL = tb["utcL"]
	-- 	local utcH = tb["utcH"]
	-- 	local utcOldL = tb["utcL"]
	-- 	local utcOldH = tb["utcH"]
	-- 	if (1 == cmpUTC(utcH - utcOldH, utcL - utcOldL, 0, 5)) then
	-- 		ok = 1
	-- 		toStoreStatus = 1
	-- 	else
	-- 		toStoreStatus = 0
	-- 	end
	-- else
	-- 	toStoreStatus = 1
	-- end

	-- 4. 
	-- for multi-AND(more than 2 rules, 
	-- return value in s2GetSelfCtrlCmd should be the same, s2GetRuleType should return (1, 1)
	-- toStoreStatus = 1
	-- if (status["pwr"] == 1) and 
	-- 	((nil == statusOld) or not (statusOld["pwr"] == 1)) then
	-- 	ok = 1
	-- end
	-- CUSTOMIZATION END

	-- print(string.format("[LUA] 0 s2apiStoreCurrStatus [%d][%s][%s] ok[%d]\r\n", toStoreStatus, uuid, statusForReserved, ok))
	if ((toStoreStatus == 1) and 
		not (nil == uuid) and 
		not (nil == statusForReserved)) then 
		-- local a = ("ok", ok)
		-- tb += a
		-- statusForReserved = cjson.encode(tb)
		local l, n = s2GetSelfName()

		-- print(string.format("[LUA] 1 s2apiStoreCurrStatus [%d][%s] \r\n", l, n))
		-- print(string.format("[LUA] 2 s2apiStoreCurrStatus [%d][%s] \r\n", string.len(uuid), uuid))
		-- print(string.format("[LUA] 3 s2apiStoreCurrStatus [%d][%s] \r\n", string.len(statusForReserved), statusForReserved))
		s2apiStoreCurrStatus(l, n, string.len(uuid), uuid, string.len(statusForReserved), statusForReserved)
		-- print(string.format("[LUA CALL] s2apiStoreCurrStatus -s\r\n"))
		-- local tblInfo = s2GetRuleInfo()
		-- for _,v in ipairs(tblInfo) do
		-- 	if uuid == v then
		-- 		s2apiStoreCurrStatus(string.len(v), v, string.len(statusForReserved), statusForReserved);
		-- 	end
		-- end
		-- print(string.format("[LUA CALL] s2apiStoreCurrStatus -e\r\n"))

		-- statusPrivious = statusForReserved
	end

	return ok
end


--[[
	EXPORT
	the self rule name (rand rule name, buf diff for every rule names)
	E.g.
	lua5.3.0_win_slim.exe qca9531/qca9531.ia.lua s2GetSelfName
]]
function s2GetSelfName()
	local name = ''
	-- CUSTOMIZATION START
	name = 'helloTest2'
	-- CUSTOMIZATION END
	return string.len(name), name
end

--[[
	EXPORT
	the self control cmd(json)
	E.g.
	lua5.3.0_win_slim.exe qca9531/qca9531.ia.lua s2GetSelfCtrlCmd
]]
function s2GetSelfCtrlCmd()
	--[[
	to get the cmd for self ctrl
	]]
	local selfCtrl = ''
	
	-- CUSTOMIZATION START
	selfCtrl = '{"ctrl":{"action":5}}'
	-- CUSTOMIZATION END
	return string.len(selfCtrl), selfCtrl
end

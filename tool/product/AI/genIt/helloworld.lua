--[[ 
	THIS IS IA SCRIPT
  ]]

--[[
	INTERNAL
]]
function cmpUTC(utcH, utcL, utcBaseH, utcBaseL) 
	local ret = -1
	if (utcH >= utcBaseH) and (utcL > utcBaseL) then
		ret = 1
	elseif (utcH == utcBaseH) and (utcL == utcBaseL) then
		ret = 0
	end

	return ret
end

--[[
	INTERNAL
]]
function cmpValInOldStatus(uuid, val, token)
	local l, n = s2GetSelfName()
	local tblOldStatus = s2apiGetLatestStatus(l, n)
	local ret = -2

	if tblOldStatus then 
		for _, jsonOldStatus in pairs(tblOldStatus) do
			if nil ~= string.find(jsonOldStatus, uuid) then
				local tbl = cjson.decode(jsonOldStatus)
				if tbl["status"][token] < val then
					ret = 1
				elseif tbl["status"][token] > val then
					ret = -1
				else
					ret = 0
				end
				break
			end
		end
	end
	return ret
end

--[[
	INTERNAL
]]
function getOldStatus(uuid)
	local l, n = s2GetSelfName()
	local tblOldStatus = s2apiGetLatestStatus(l, n)

	if tblOldStatus then 
		for _, jsonOldStatus in pairs(tblOldStatus) do
			if nil ~= string.find(jsonOldStatus, uuid) then
				return jsonOldStatus
			end
		end
	end
	return nil
end

--[[
	EXPORT
]]
function getBeingReservedInfo()
	-- CUSTOMIZATION START
	local tblInfo = {'10000100091000610006FFFFFFFFFFFF', '10000100101000010007FFFFFFFFFFFF'}
	-- CUSTOMIZATION END
	return tblInfo
end

--[[
	EXPORT
	if the rule has been expired
	selfStatus: json (S2 json format) => {\"status\":{},\"utcH\":339,\"utcL\":136006835}
	E.g. utc:1456129920 (2016/2/22 16:32:00(8+))      utcH: (339)0x153 utcL: 136006835(0x081b4cb3)
	lua5.3.0_win_slim.exe hello/helloworld.lua s2IsValid {\"status\":{},\"utcH\":339,\"utcL\":136006835}
]]
function s2IsValid(selfStatus)
	local valid = 0
	local tb = cjson.decode(selfStatus)
	local utcL = tb["utcL"]
	local utcH = tb["utcH"]
	
	-- E.g. 16/12/1 0:00:00 is 0x583F6800
	-- CUSTOMIZATION START
	local utcBaseH = 0
	local utcBaseL = 0x583F6800
	-- CUSTOMIZATION END
	if (cmpUTC(utcH, utcL, utcBaseH, utcBaseL) >= 0) then 
		valid = 1
	end
	return valid
end

--[[
	EXPORT
	return value: [repeating call or status trig, AND or OR]
	lua5.3.0_win_slim.exe hello/helloworld.lua s2GetRuleType
]]
function s2GetRuleType()
	local repeatOrTrig = 0
	local isAND = 0
	-- repeating call or status trig, AND or OR
	-- CUSTOMIZATION START
	repeatOrTrig = 0
	isAND = 0
	-- CUSTOMIZATION END
	return repeatOrTrig, isAND
end

--[[
	EXPORT
	the being reserved node info(uuid(s))
	lua5.3.0_win_slim.exe hello/helloworld.lua s2GetBeingReservedInfo
]]
function s2GetBeingReservedInfo()
	local reserved = ''
	local count = 0
	local tblInfo = getBeingReservedInfo()
	return tblInfo
end


function s2IsConditionOKExt(selfStatus, rmtStatus)
	local ok = 0
	local toStoreStatus = 0
	local rmtUUID = nil
	local tblRmt = {}
	if not (nil == rmtStatus) then
		tblRmt = cjson.decode(rmtStatus)
	end
	local tblSelf = {}
	if not (nil == selfStatus) then
		tblSelf = cjson.decode(selfStatus)
	end

	r, a = s2GetRuleType()
	local rmtUUID = tblRmt["uuid"]
	if nil == rmtUUID and 0 == r then
		return ok
	end

	-- CUSTOMIZATION START
	toStoreStatus = 1

	local tblOldStatus = {}
	local tblConditions = {}
	local tblConditionsOld = {}
	local retIdx = 1;
	local tblNewStatus = tblRmt["status"]
	if nil == tblNewStatus then
		return ok
	end

	-- CONDITION(s) START
	tblConditions[retIdx] = 0
	tblConditionsOld[retIdx] = 0
	if nil ~= string.find(rmtUUID, '10000100091000610006FFFFFFFFFFFF') then
		if 100 == tblNewStatus["percentage"] and 0 ~= cmpValInOldStatus('10000100091000610006FFFFFFFFFFFF', tblNewStatus["percentage"], "percentage") then
			tblConditions[retIdx] = 1
			print('ok 1\r\n')
		end
	else
		if 0 == cmpValInOldStatus('10000100091000610006FFFFFFFFFFFF', 100, "percentage") then
			tblConditionsOld[retIdx] = 1
			print('okOld 1\r\n')
		end
	end
	retIdx = retIdx + 1

	tblConditions[retIdx] = 0
	tblConditionsOld[retIdx] = 0
	if nil ~= string.find(rmtUUID, '10000100101000010007FFFFFFFFFFFF') then
		-- print('check 1\r\n')
		if 2 == tblNewStatus["idx3"] and 0 ~= cmpValInOldStatus('10000100101000010007FFFFFFFFFFFF', tblNewStatus["idx3"], "idx3") then
			tblConditions[retIdx] = 1
			print('ok 2\r\n')
		end
	else
		if 0 == cmpValInOldStatus('10000100101000010007FFFFFFFFFFFF', 2, "idx3") then
			tblConditionsOld[retIdx] = 1
			print('okOld 2\r\n')
		end
	end
	retIdx = retIdx + 1

	-- CONDITION END

	--[[
		$(CONDITIONS CHECK) may include the following cases
	]]
	-- 1. OR condition check 
	if tblConditions[1] or tblConditions[2] then
		ok = 1
	end

	-- -- 2. AND condition check 
	-- if 1 == tblConditions[1] and 1 == tblConditionsOld[2] then
	-- 	ok = 1
	-- end
	-- if 1 == tblConditions[2] and 1 == tblConditionsOld[1] then
	-- 	ok = 1
	-- end



	-- CUSTOMIZATION END

	if ((toStoreStatus == 1) and 
	not (nil == rmtUUID) and 
	not (nil == rmtStatus)) then 
		local l, n = s2GetSelfName()
		s2apiSetCurrStatus(l, n, string.len(rmtUUID), rmtUUID, string.len(rmtStatus), rmtStatus)
	end

	return ok
end


--[[
	EXPORT
	the self rule name (rand rule name, buf diff for every rule names)
	E.g.
	lua5.3.0_win_slim.exe hello/helloworld.lua s2GetSelfName
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
	lua5.3.0_win_slim.exe hello/helloworld.lua s2GetSelfCtrlCmd
]]
function s2GetSelfCtrlCmd()
	--[[
		to get the cmd for self ctrl
	]]
	local selfCtrl = ''
	
	-- CUSTOMIZATION START
	selfCtrl = '{"ctrl":{"idx3":2}}'
	-- CUSTOMIZATION END
	return string.len(selfCtrl), selfCtrl
end

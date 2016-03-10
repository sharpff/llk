--[[ 
	THIS IS IA SCRIPT
  ]]

--[[
	INTERNAL
]]
function cmpUTC(utcH, utcL, utcBaseH, utcBaseL) 
	local ret = -1
	if (utcH >= utcBaseH) and (utcL > utcBaseL) then
		print ("abc\r\n")
		ret = 1
	elseif (utcH == utcBaseH) and (utcL == utcBaseL) then
		print ("cba\r\n")
		ret = 0
	end
	print ("aaa\r\n")

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
		print(string.format("[LUA] ==> WORKING ON tblOldStatus[%d]\r\n", #tblOldStatus))
		for _, jsonOldStatus in pairs(tblOldStatus) do
			print('cmpValInOldStatus '..token..' => '..jsonOldStatus..'\r\n')  	
			if nil ~= string.find(jsonOldStatus, uuid) then
				local tbl = cjson.decode(jsonOldStatus)
				if tbl["status"][token] < val then
					ret = 1
				elseif tbl["status"][token] > val then
					ret = -1
				else
					ret = 0
				end
				print('cmpValInOldStatus => '..ret..'\r\n')  	
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
	-- print(string.format("[LUA] getOldStatus [%d][%s] \r\n", l, n))

	if tblOldStatus then 
		for _, jsonOldStatus in pairs(tblOldStatus) do
			if nil ~= string.find(jsonOldStatus, uuid) then
				print('getOldStatus GOT => '..jsonOldStatus..'\r\n')
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
	lua5.3.0_win_slim.exe qca9531/ia_forQCA9531.lua s2IsValid {\"status\":{},\"utcH\":339,\"utcL\":136006835}
]]
function s2IsValid(selfStatus)
	local valid = 0
	-- print (selfStatus)
	local tb = cjson.decode(selfStatus)
	local utcL = tb["utcL"]
	local utcH = tb["utcH"]
	
	-- print(string.format("[LUA] selfStatus [%d][%s] \r\n", string.len(selfStatus), selfStatus))

	-- CUSTOMIZATION START
	local utcBaseH = 0x081b4cb3
	local utcBaseL = 0x153
	-- CUSTOMIZATION END
	-- if (cmpUTC(utcH, utcL, utcBaseH, utcBaseL) == 1) then 
		valid = 1
	-- end
	return valid
end

--[[
	EXPORT
	return value: [repeating call or status trig, AND or OR]
	lua5.3.0_win_slim.exe qca9531/ia_forQCA9531.lua s2GetRuleType
]]
function s2GetRuleType()
	local repeatOrTrig = 0
	local isAND = 0
	-- repeating call or status trig, AND or OR
	-- CUSTOMIZATION START
	repeatOrTrig = 1
	isAND = 0
	-- CUSTOMIZATION END
	return repeatOrTrig, isAND
end

--[[
	EXPORT
	the being reserved node info(uuid(s))
	lua5.3.0_win_slim.exe qca9531/qca9531.ia.lua s2GetBeingReservedInfo
]]
function s2GetBeingReservedInfo()
	local reserved = ''
	local count = 0
	local tblInfo = getBeingReservedInfo()
	-- print(string.format("[LUA] s2GetBeingReservedInfo tblInfo[%d]\r\n", #tblInfo))
	return tblInfo
end


function s2IsConditionOKExt(selfStatus, rmtStatus)
	local ok = 0
	local toStoreStatus = 0
	-- print ('[LUA] s2IsConditionOKExt -s\r\n')
	-- print(string.format("[LUA] s2IsConditionOKExt selfStatus[%s]\r\n", selfStatus))
	-- print(string.format("[LUA] s2IsConditionOKExt rmtStatus[%s]\r\n", rmtStatus))
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
		print ('[LUA] s2IsConditionOKExt -e1\r\n')
		return ok
	end
	local uuidSelf = tblSelf["uuid"]
	if nil == uuidSelf then
		print ('[LUA] s2IsConditionOKExt -e2\r\n')
		return ok
	end

	-- CUSTOMIZATION START

	-- 2. for status & time
	toStoreStatus = 1
	local uuid1 = '10000100091000610006FFFFFFFFFFFF'
	local token1 = "percentage"
	local cmpVal1 = 101
	local ok = 0
	-- (2016/2/22 16:32:00(8+))
	local utcSpanH = 0x0
	local utcSpanL = 0x14 -- 5 secss
	local utcSelfL = tblSelf["utcL"]
	local utcSelfH = tblSelf["utcH"]
	-- invalid utc, it may no passed the auth
	if 0 >= utcSelfL then
		return ok
	end 

	local newNodeVal1 = nil
	local tblOldStatus = nil
	local jsonOldStatus = getOldStatus(uuid1)

	if jsonOldStatus or (rmtUUID and nil ~= string.find(rmtUUID, uuid1)) then
		local tblNewStatus = tblRmt["status"]
		-- to get the old val
		if nil == tblNewStatus then
			-- print ('[LUA] a1 \r\n')
			if jsonOldStatus then
				-- print ('[LUA] a2 \r\n')
				tblOldStatus = cjson.decode(jsonOldStatus)
				if tblOldStatus then
					newNodeVal1 = tblOldStatus["status"][token1]
					-- print ('[LUA] a3 '..'token is '..token1..'\r\n')
					print ('[LUA] a33 val is '..newNodeVal1..'\r\n')
				end
			end
		else
			-- to get the new val
			newNodeVal1 = tblNewStatus[token1]
			print ('[LUA] a4 \r\n')
		end

		if newNodeVal1 then
			-- print(string.format("[LUA] ==> a5 newNodeVal1[%d]\r\n", newNodeVal1))
			if cmpVal1 < newNodeVal1 then 
				print('check 1\r\n')
				if tblOldStatus and 0 <= cmpUTC(utcSelfH - tblOldStatus["utcH"], utcSelfL - tblOldStatus["utcL"], utcSpanH, utcSpanL) then
					-- to store the utc to old status
					print ('[LUA] a7 \r\n')
					tblRmt = tblOldStatus
					rmtUUID = tblRmt["uuid"]
					tblRmt["utcL"] = utcSelfL
					tblRmt["utcH"] = utcSelfH
					rmtStatus = cjson.encode(tblRmt)
					toStoreStatus = 1
					ok = 1
				end
			else
				-- to store the utc to old status
				print ('[LUA] a8 \r\n')
				tblRmt["utcL"] = utcSelfL
				tblRmt["utcH"] = utcSelfH
				rmtStatus = cjson.encode(tblRmt)
				toStoreStatus = 1
			end
		end
	end

	-- CUSTOMIZATION END

	if ((toStoreStatus == 1) and 
	not (nil == rmtUUID) and 
	not (nil == rmtStatus)) then 
		local l, n = s2GetSelfName()
		print(string.format("[LUA] s2apiSetCurrStatus [%d][%s] => [%s][%s] \r\n", l, n, rmtUUID, rmtStatus))
		s2apiSetCurrStatus(l, n, string.len(rmtUUID), rmtUUID, string.len(rmtStatus), rmtStatus)
	end

	-- print(string.format("[LUA] s2IsConditionOKExt[%d] -e\r\n", ok))
	return ok
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
	print(string.format("[LUA] statusForReserved [%d][%s] \r\n", string.len(statusForReserved), statusForReserved))
	print(string.format("[LUA] statusPrivious [%d][%s] \r\n", string.len(statusPrivious), statusPrivious))
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
	-- toStoreStatus = 1
	-- if (not (nil == status) and (status["idx2"] == 1)) and
	-- 	((nil == statusOld) or not (statusOld["idx2"] == 1)) then
	-- 	ok = 1
	-- end

	-- 2. 
	-- for single AND
	-- toStoreStatus = 1
	-- if (nil ~= status and status["idx2"] == 1 and status["idx3"] == 1 and
	-- 	(nil == statusOld["idx2"] or nil == statusOld["idx3"] or 1 ~= statusOld["idx2"] or 1 ~= statusOld["idx3"])) then
	-- 	ok = 1
	-- end
	
	-- 3.
	-- for multiple source AND
	toStoreStatus = 1
	if nil ~= string.find(uuid, '10000100091000610006FFFFFFFFFFFF') then
	end


	-- 4. 
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
	-- CUSTOMIZATION END


	-- print(string.format("[LUA] s2IsConditionOK [%d][%s][%s] ok[%d]\r\n", toStoreStatus, uuid, statusForReserved, ok))
	if ((toStoreStatus == 1) and 
		not (nil == uuid) and 
		not (nil == statusForReserved)) then 
		-- local a = ("ok", ok)
		-- tb += a
		-- statusForReserved = cjson.encode(tb)
		local l, n = s2GetSelfName()

		-- print(string.format("[LUA] 1 s2apiSetCurrStatus [%d][%s] \r\n", l, n))
		-- print(string.format("[LUA] 2 s2apiSetCurrStatus [%d][%s] \r\n", string.len(uuid), uuid))
		-- print(string.format("[LUA] 3 s2apiSetCurrStatus [%d][%s] \r\n", string.len(statusForReserved), statusForReserved))
		s2apiSetCurrStatus(l, n, string.len(uuid), uuid, string.len(statusForReserved), statusForReserved)
		-- print(string.format("[LUA CALL] s2apiSetCurrStatus -s\r\n"))
		-- local tblInfo = s2GetRuleInfo()
		-- for _,v in ipairs(tblInfo) do
		-- 	if uuid == v then
		-- 		s2apiSetCurrStatus(string.len(v), v, string.len(statusForReserved), statusForReserved);
		-- 	end
		-- end
		-- print(string.format("[LUA CALL] s2apiSetCurrStatus -e\r\n"))

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
	selfCtrl = '{"ctrl":{"action":4}}'
	-- selfCtrl = '{"ctrl":{"action":1}}'
	-- CUSTOMIZATION END
	return string.len(selfCtrl), selfCtrl
end

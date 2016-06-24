--[[ 
	THIS IS FW SCRIPT for foree
  ]]
--[[ INTERNAL
	Table to string
  ]]
function tableToString( cmd )
	local strcmd = ""
	local i
	
	for i=1, #cmd do
		strcmd = strcmd .. string.char(cmd[i])
	end
	return strcmd
end

--[[ INTERNAL
	String to table
  ]]
function stringToTable( sta )
	local tablesta = {}
	local i
	for i=1, #sta do
		tablesta[i] = sta:byte(i)
	end

	return tablesta
end

--[[ INTERNAL
	log table info
  ]]
function LOGTBL(tblObj)
	local str = ""
	for i = 1, #tblObj do 
		str = str..string.format('%02x ', tblObj[i])
	end
	print ('LOGTBL '..str..'\r\n')
end

--[[ INTERNAL
	fast CRC8
  ]]
function fastCRC(lastCRC, newByte)
	local crcTbl = {0, 0x07, 0x0E, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D}
	local crcTbl2 = {0, 0x70, 0xE0, 0x90, 0xC1, 0xB1, 0x21, 0x51, 0x83, 0xF3, 0x63, 0x13, 0x42, 0x32, 0xA2, 0xD2}
	local idx = newByte
	idx = idx ~ lastCRC
	idx = idx >> 4
	lastCRC = lastCRC & 0x0F
	lastCRC = lastCRC ~ crcTbl2[idx + 1]
	-- local stridx = string.format('%02x ', idx)
	-- local strLastCRC = string.format('%02x ', lastCRC)
	-- print ("[LUA] fastCRC int is "..stridx.." "..strLastCRC.."\r\n")
	idx = lastCRC
	idx = idx ~ newByte
	idx = idx & 0x0F
	lastCRC = lastCRC & 0xF0
	lastCRC = lastCRC ~ crcTbl[idx + 1]
	return lastCRC
end

--[[ INTERNAL
	crc8
  ]]
function crc8(data)
	local pec = 0x00
	-- # data = struct.unpack('i', data)
	-- for byte in data:
	for _, byte in ipairs(data) do
		-- byte = ord(byte)
		local str = string.format('%02x ', byte)
		-- print ("[LUA] byte "..str.."\r\n")
		pec = (fastCRC(pec, byte))
		-- str = string.format('%02x ', pec)
		-- print ("[LUA] fastCRC "..str.."\r\n")
	end
	return pec
end

-- Cluster ID:
-- 1) 0x0000: 基本类
-- 2) 0x0006：ON/OFF类
-- 3) 0x0008：Level类
-- 4) 0x0001：电源管理类
-- 5) 0x0406：PIR，人体识别类
-- 6) 0x0402：温度测量类
-- 7) 0x0405：湿度测量类
-- 8) 0x0201: 温控器类
function getClusterFromDid(did)
	-- print ("did is "..did.."\r\n")
	if nil ~= string.find(did, "0000") then
		return string.format('%02x%02x', 0x00, 0x06)
	elseif nil ~= string.find(did, "0107") then
		return string.format('%02x%02x', 0x04, 0x06)
	end
end
--[[ EXTERNAL
	s1GetVer
  ]]
function s1GetVer()
	-- local cmdTbl = {0xAA, 0x00, 0x04, 0x02, 0x00, 0x00, 0x00}
	-- local ret = crc8(cmdTbl)
	-- local str = string.format('%02x ', ret)
	-- print ("[LUA] crc8 "..str.."\r\n")

	-- body
	local str = '1.0'
	return string.len(str), str
end

--[[ EXTERNAL
	s1GetCvtType
  ]]
function s1GetCvtType()
    -- combained uart(0x1) & gpio(0x2)
    local str = [[
    {
    "whatCvtType":1,
    "uart":[
		{
			"id":1, 
			"baud":"115200-8N1"
		}
    	]
	}
    ]]
	local delay = 5

	return string.len(str), str, delay
end

--[[ EXTERNAL
	s1GetQueries
  ]]
function s1GetQueries(queryType)
	local cvtType = s1apiGetCurrCvtType()
	local query = ''
	local queryCountLen = ''

	-- print ("[LUA] s1GetQueries cvtType is " .. cvtType .. ", queryType is " .. queryType .."\r\n")

	for i = 1, 1 do
		-- UART
		if 0x01 == cvtType then

			-- status for itself
			tmpType = (queryType & 0x0000FFFF)
			if 0 ~= tmpType then
				-- print ("[LUA] status for itself \r\n")
				-- if tmpType == 1 then
				-- 	print ("[LUA] tmpType is "..tmpType.."\r\n")
				-- elseif tmpType == 2 then
				-- 	print ("[LUA] tmpType is "..tmpType.."\r\n")
				-- end
				break
			end

			-- status for sub devs
			tmpType = ((queryType & 0xFFFF0000) >> 16)
			if 0 ~= tmpType then
				print ("[LUA] status for sub devs\r\n")
				-- if tmpType == 1 then
				-- 	print ("[LUA] tmpType is "..tmpType.."\r\n")
				-- elseif tmpType == 2 then
				-- 	print ("[LUA] tmpType is "..tmpType.."\r\n")
				-- end
				break
			end


		end

		-- GPIO
		if 0x02 == cvtType then

		end
	end

	return string.len( queryCountLen ), queryCountLen, string.len( query ), query
end

function s1OptHasSubDevs()
	return 1
end

--[[ OPTIONAL
	s1OptDoSplit
  ]]
function s1OptDoSplit(data)
	local tblData = stringToTable(data)
	local tblDataCountLen = {}
	local strDataCountLen = ""
	local where = 1
	local singleData = nil
	local idx = 0
	-- print("total is "..#tblData.."\r\n")
	while where < #tblData do
		-- print("span is "..where..":"..(where + 3 + tblData[where + 2] + 1).."\r\n")
		local tmpString = string.sub(data, where, (where + 3 + tblData[where + 2] - 1))
		local tmpTbl = stringToTable(tmpString)
		-- local cmp = crc8(tmpTbl)
		-- print ("crc8 => "..cmp..", tblData => "..tblData[(where + 3 + tblData[where + 2])].."\r\n")
		if nil == tblData[where + 2] or (#tblData - where + 1) < (3 + tblData[where + 2] + 1) or 
			crc8(tmpTbl) ~= tblData[(where + 3 + tblData[where + 2])]then
			-- print("break1\r\n")
			break
		end
		-- print("xxx is "..where..", 1st is "..(#tblData - where + 1)..", 2nd is "..(3 + tblData[where + 2] + 1).." => ")
		-- LOGTBL(tmpTbl)
		tblDataCountLen[idx + 1] = (3 + tblData[where + 2] + 1) & 0xFF
		-- tblDataCountLen[idx + 2] = ((3 + tblData[where + 2] + 1) >> 8) & 0xFF
		tblDataCountLen[idx + 2] = 0x00
		idx = idx + 2
		where = where + (3 + tblData[where + 2] + 1)
	end

	strDataCountLen = tableToString(tblDataCountLen)
	-- LOGTBL(tblDataCountLen)

	-- print(string.format('count [%d] ', string.len( strDataCountLen)) .. LOGTBL(stringToTable(strDataCountLen)))

	return string.len( strDataCountLen ), strDataCountLen, string.len( data ), data
end

function s1OptMergeCurrStatus2Action(action, currStatus)
	return string.len(action), action
end

--[[ EXTERNAL
	s1GetValidKind
  ]]
function s1GetValidKind(data)
	local cvtType = s1apiGetCurrCvtType()
	local ret = 0 -- invalid kind
	local dataTbl = nil
	local WHATKIND_MAIN_DEV_RESET = 1
	local WHATKIND_MAIN_DEV_DATA = 2
	local WHATKIND_SUB_DEV_RESET = 10
	local WHATKIND_SUB_DEV_DATA = 11
	local WHATKIND_SUB_DEV_JOIN = 12
	local WHATKIND_SUB_DEV_LEAVE = 13

	local tmp = stringToTable(data)
	LOGTBL(tmp)

	for i = 1, 1 do
		-- UART
		if 0x01 == cvtType then
			dataTbl = stringToTable(data)
			-- START for status of sub devices
			if nil ~= string.find(data, string.char(0xAA, 0x00, 0x05, 0x82, 0x00, 0x00, 0x00, 0x00, 0xB1, 0xFF, 0xFF)) then
				-- (RSP) reset FAC rsp , rep is AA 00 04 02 00 00 00 DB
				print ("[LUA] s1GetValidKind - sub devices - reset rsp\r\n")
				ret = WHATKIND_SUB_DEV_RESET
				break
			end

			if nil ~= string.find(data, string.char(0xAA, 0x00, 0x05, 0x82, 0x04, 0x01, 0x41, 0x00, 0xCC)) then
				-- (RSP) join permition rsp , rep is AA 00 0D 02 04 01 41 00 00 00 00 00 00 00 00 00 7B
				print ("[LUA] s1GetValidKind - sub devices - join permition rsp\r\n")
				ret = WHATKIND_SUB_DEV_DATA
				break
			end

			if nil ~= string.find(data, string.char(0xAA, 0x00, 0x13, 0x81, 0x10)) then
				-- (RSP) list rsp , rep is AA 00 02 01 10 9D
				print ("[LUA] s1GetValidKind - sub devices - list rsp\r\n")
				ret = WHATKIND_SUB_DEV_DATA
				break
			end

			if dataTbl[1] == 0xAA and dataTbl[2] == 0x00 and dataTbl[4] == 0x81 and dataTbl[5] == 0x00 then
				-- (RSP) device info rsp , rep is AA 00 04 01 00 00 00 E1
				print ("[LUA] s1GetValidKind - sub devices - device info rsp "..#dataTbl.."\r\n")
				ret = WHATKIND_SUB_DEV_DATA
				break
			end

			if nil ~= string.find(data, string.char(0xAA, 0x00, 0x11, 0x82)) then
				-- (IND) new device joining, rep is 0xAA, 0x00, 0x11, 0x82
				print ("[LUA] s1GetValidKind - sub devices - new device joining\r\n")
				ret = WHATKIND_SUB_DEV_JOIN
				break
			end

			if dataTbl[1] == 0xAA and dataTbl[2] == 0x00 and dataTbl[4] == 0x90 then
				-- (IND) sDevStatus action ind
				print ("[LUA] s1GetValidKind - sub devices - sDevStatus action ind "..#dataTbl.."\r\n")
				ret = WHATKIND_SUB_DEV_DATA
				break
			end

		end

		-- GPIO
		if 0x02 == cvtType then
			return WHATKIND_MAIN_DEV_DATA
		end
	end
	-- invalid kind
	return ret
end

-- {"msg":"hello","sDev":{"pid":"0104","did":"0107","clu":"0006","ept":[1, 2],"mac":"7409E17E3376AF60"}}
-- {"ctrl":{"pwr":1,"sDev":{"pid":"0104","did":"0107","clu":"0006","ept":[1, 2],"mac":"7409E17E3376AF60"}}}
-- {"status":{"pwr":1,"switcher":1,"sDev":{"pid":"0104","did":"0107","clu":"0006","ept":[1, 2],"mac":"7409E17E3376AF60"}}}
-- s1apiSDevGetMacByUserData()
-- s1apiSdevGetUserDataByMac()

-- \{\"ctrl\":\{\"reset\":1\}\}
-- \{\"ctrl\":\{\"sDevJoin\":1\}\}
-- \{\"ctrl\":\{\"sDevGetList\":1\}\}
-- \{\"ctrl\":\{\"sDevGetInfo\":0\}\}
-- \{\"ctrl\":\{\"sDevGetInfo\":1\}\}
-- \{\"ctrl\":\{\"pwr\":1,\"sDev\":\{\"pid\":\"0104\",\"did\":\"0107\",\"clu\":\"0006\",\"ept\":[1,2],\"mac\":\"7409E17E3376AF60\"\}\}\}

--[[ EXTERNAL
	s1CvtStd2Pri
  ]]
function s1CvtStd2Pri(json)
	local cvtType = s1apiGetCurrCvtType()
	print ('[LUA] s1CvtStd2Pri return => '..json..'\r\n')
	local ctrl = cjson.decode(json)
	-- local sDev = ctrl["sDev"]
	local cmdTbl = {}
	local dataStr = ""

	for i = 1, 1 do
		-- UART
		if 0x01 == cvtType then
			if ctrl["reset"] == 1 then
				cmdTbl = {0xAA, 0x00, 0x04, 0x02, 0x00, 0x00, 0x00, 0xDB}
				break
			end

			if ctrl["sDevJoin"] == 1 then
				cmdTbl = {0xAA, 0x00, 0x0D, 0x02, 0x04, 0x01, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7B}
 				break
			end

			if ctrl["sDevGetList"] == 1 then
				cmdTbl = {0xAA, 0x00, 0x02, 0x01, 0x10, 0x9D}
				break
			end

			if ctrl["sDevGetInfo"] then
				print ('[LUA] sDevGetInfo => '..json..'\r\n')
				local idx = ctrl["sDevGetInfo"]
				-- idx = s1apiSdevGetUserDataByMac(string.len(ctrl["sDevGetInfo"]), ctrl["sDevGetInfo"])
				if 0 <= idx then
					cmdTbl = {0xAA, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00}
					cmdTbl[6] = idx
					cmdTbl[8] = crc8(cmdTbl)
				end
				break
			end
			-- TODO: ctrl the sub dev.
			if sDev then 
				print("TODO: ctrl the sub dev. \r\n")
			end
		end
		-- GPIO
		if 0x02 == cvtType then
		end
	end

	LOGTBL(cmdTbl)
	dataStr = tableToString(cmdTbl)
	return string.len(dataStr), dataStr
end

function genStatus(eptList, len, data)
	local status = "{}"
	for _, dept in ipairs(eptList) do
		-- print("did "..dept[1]..", ept "..dept[2].."\r\n")
		if nil ~= string.find(dept[1], "0000") then 
			status = string.format('{"switcher":%d}', data[4])
		elseif nil ~= string.find(dept[1], "0107") then
			status = string.format('{"detector":%d}', data[4])
		end
	end
	return status
end
--[[ EXTERNAL
	s1CvtPri2Std
  ]]
function s1CvtPri2Std(bin)
	local cvtType = s1apiGetCurrCvtType()
	local dataTbl = {}
	local strMain = ''
	local strSubDev = '"sDev":{"pid":"%s","clu":"%s","ept":%s,"mac":"%s"}'
	dataTbl = stringToTable(bin)

	for i = 1, 1 do
		-- UART
		if 0x01 == cvtType then
			-- START for status of sub devices
			if nil ~= string.find(bin, string.char(0xAA, 0x00, 0x05, 0x82, 0x04, 0x01, 0x41, 0x00, 0xCC)) then
				-- (RSP) join permition rsp , rep is AA 00 0D 02 04 01 41 00 00 00 00 00 00 00 00 00 7B
				print ("[LUA] s1CvtPri2Std - sub devices - join permition rsp\r\n")
				strMain = string.format('{"sDevJoin":%d}', 2)
				break
			end

			if nil ~= string.find(bin, string.char(0xAA, 0x00, 0x13, 0x81, 0x10)) then
				-- (RSP) list rsp , rep is AA 00 02 01 10 9D
				-- {"sDevGetList":[0,1,2]}
				print ("[LUA] s1CvtPri2Std - sub devices - list rsp\r\n")
				local num = 0
				local sDevList = {}
				for idx = 7, 22 do
					-- print(idx.."\r\n")
					if 0x00 ~= dataTbl[idx] then
						for m = 0, 7 do
							local tmp = (dataTbl[idx] >> m) & 0xFF
							-- print("mask "..tmp.."\r\n")
							if 0 ~= tmp then
								num = num + 1
								sDevList[#sDevList + 1] = 8*(idx - 7) + m
								-- print("idx is "..testIdx.."\r\n")
							end
						end
					end
				end
				-- for n = 1, #sDevList do 
				-- 	print("sDevList["..n.."] is "..sDevList[n].."\r\n")
				-- end
				local s = cjson.encode(sDevList)
				strMain = '{"sDevGetList":'..s..'}'
				-- string.format('{"sDevGetList":}', num)
				print("[LUA] return => "..strMain.."\r\n")
				-- print(s.."\r\n")
				break
			end

			if dataTbl[1] == 0xAA and dataTbl[2] == 0x00 and dataTbl[4] == 0x81 and dataTbl[5] == 0x00 then
				-- (RSP) device info rsp , rep is AA 00 04 01 00 00 00 E1
				-- {"sDevGetInfo":indexVal,"sDev":{"pid":"0401","clu":"0107","ept":[["0000",1],["0000",2],["0000",3]],"mac":"7409E17E3376AF60"}}
				print ("[LUA] s1CvtPri2Std - sub devices - device info rsp "..#dataTbl.."\r\n")
				local strMac = string.format("%02X%02X%02X%02X%02X%02X%02X%02X", dataTbl[9], dataTbl[10], dataTbl[11], dataTbl[12], dataTbl[13], dataTbl[14], dataTbl[15], dataTbl[16])
				local strProId = string.format("%02x%02x", dataTbl[18], dataTbl[17])
				local devNumIdx = 26
				local devNum = dataTbl[devNumIdx]
				print("devNum is "..devNum.." strProId is "..strProId.."\r\n")
				local sDevEPList = {}
				local strCluster = '""'
				for i = 1, devNum do
					local strDevId = string.format("%02x%02x", dataTbl[(devNumIdx + 1) + 3*(i - 1) + 1], dataTbl[(devNumIdx + 1) + 3*(i - 1)])
					sDevEPList[#sDevEPList + 1] = {strDevId, dataTbl[(devNumIdx + 1) + 3*(i - 1) + 2]}
					strCluster = getClusterFromDid(strDevId)
				end
				local ept = cjson.encode(sDevEPList)
				strSubDev = string.format(strSubDev, strProId, strCluster, ept, strMac)
				strMain = '{"sDevGetInfo":'..dataTbl[6]..','..strSubDev..'}'
				-- print(ept.."\r\n")
				print("[LUA] return =====> "..strMain.."\r\n")
				break
			end

			if nil ~= string.find(bin, string.char(0xAA, 0x00, 0x11, 0x82)) then
				-- (IND) new device joining, rep is 0xAA, 0x00, 0x11, 0x82
				-- {"sDevJoin":2,"sDev":{"pid":"0401","clu":"0107","ept":[["0701",1]],"mac":"6FE34CE400A06FC0"}}
				print("s1CvtPri2Std - sub devices - new device joining\r\n")
				local strProId = string.format("%02x%02x", dataTbl[6], dataTbl[5])
				local addr = dataTbl[8]
				local strMac = string.format("%02X%02X%02X%02X%02X%02X%02X%02X", dataTbl[11], dataTbl[12], dataTbl[13], dataTbl[14], dataTbl[15], dataTbl[16], dataTbl[17], dataTbl[18])
				local sDevEPList = {}
				local strCluster = '""'
				for i = 1, 1 do
					local strDevId = string.format("%02x%02x", dataTbl[20], dataTbl[19])
					sDevEPList[#sDevEPList + 1] = {strDevId, 1}
					strCluster = getClusterFromDid(strDevId)
				end
				local ept = cjson.encode(sDevEPList)
				strSubDev = string.format(strSubDev, strProId, strCluster, ept, strMac)
				strMain = '{"sDevJoin":2,'..strSubDev..'}'
				print("[LUA] return => "..strMain.."\r\n")
				break
			end

			if dataTbl[1] == 0xAA and dataTbl[2] == 0x00 and dataTbl[4] == 0x90 then
				-- (IND) sDevStatus action ind
				-- {"sDevStatus":{"btn":1},"sDev":{"pid":"0401","clu":"0000","ept":[["0000",5]],"mac":"7409E17E3376AF60"}}
				-- {"sDevStatus":{"act":1},"sDev":{"pid":"0401","clu":"0107","ept":[["0701",1]],"mac":"6FE34CE400A06FC0"}}
				print ("[LUA] s1CvtPri2Std - sub devices - sDevStatus action ind "..#dataTbl.."\r\n")
				local strProId = string.format("%02x%02x", dataTbl[6], dataTbl[5])
				local addr = dataTbl[10]
				-- local lenMac = 0
				-- local strMac = ""
				local lenMac, strMac = s1apiSDevGetMacByUserData(addr)
				-- print("[LUA] lenMac is "..lenMac..", strMac is "..strMac.."\r\n")
				local sDevEPList = {}
				local strCluster = '""'
				for i = 1, 1 do
					local strDevId = string.format("%02x%02x", dataTbl[13], dataTbl[12])
					sDevEPList[#sDevEPList + 1] = {strDevId, dataTbl[11]}
					strCluster = getClusterFromDid(strDevId)
				end
				local ept = cjson.encode(sDevEPList)
				strSubDev = string.format(strSubDev, strProId, strCluster, ept, strMac)
				local tblStatusData = {}
				for i = 15, (dataTbl[14] + 15) do
					tblStatusData[#tblStatusData + 1] = dataTbl[i]
				end
				strMain = '{"sDevStatus":'..genStatus(sDevEPList, dataTbl[14], tblStatusData)..','..strSubDev..'}'
				print("[LUA] return => "..strMain.."\r\n")
				break
			end

		end

		-- GPIO
		if 0x02 == cvtType then
			return WHATKIND_MAIN_DEV_DATA
		end
	end
	return string.len(strMain), strMain
end

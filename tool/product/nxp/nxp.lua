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

function csum(data)
    local u8CRC;
    local tmpData = {}
    local u16Type = 0x0000
    u16Type = (data[2] << 8) | data[3]
    -- print (u16Type..'\r\n')
	for i = 7, #data - 1 do
		tmpData[i - 6] = data[i]
	end

    -- print (#tmpData..'\r\n')
    u8CRC  = (u16Type >> 0) & 0xff
    u8CRC = u8CRC ~ ((u16Type >> 8) & 0xff)
    u8CRC = u8CRC ~ ((#tmpData >> 0) & 0xff)
    u8CRC = u8CRC ~ ((#tmpData >> 8) & 0xff)

	for _, byte in ipairs(tmpData) do
        u8CRC = u8CRC ~ byte
	end

    return u8CRC
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

function whatWrite(tblData)
	local tmpTbl = {}
	local j = 1
	for i = 1, #tblData do
		if 0x10 > tblData[i] and 1 ~= i and #tblData ~= i then
			tmpTbl[j] = 0x02
			j = j + 1
			tmpTbl[j] = 0x10 | tblData[i]
		else
			tmpTbl[j] = tblData[i]
		end
		j = j + 1
	end
	return tmpTbl
end

function whatRead(tblData)
	local tmpTbl = {}
	local j = 1
	local flag = 0
	for i = 1, #tblData do
		if 0x02 == tblData[i] and 1 ~= i and #tblData ~= i then
			tmpTbl[j] = 0x0F & tblData[i + 1]
			flag = 1
			j = j + 1
		else
			if 1 == flag then
				flag = 0
			else 
				tmpTbl[j] = tblData[i]
				j = j + 1
			end
		end
	end
	return tmpTbl
end

--[[ EXTERNAL
	s1GetVer
  ]]
function s1GetVer()
	-- local cmdTbl = {0x01, 0x81, 0x02, 0x00, 0x0B, 0x7F, 0x07, 0xCA, 0x28, 0x05, 0x00, 0x06, 0x00, 0x00, 0x00, 0x10, 0x01, 0x03}
	-- local ret = csum(cmdTbl)
	-- local str = string.format('%02X ', ret)
	-- print ("[LUA] csum "..str.."\r\n")

	local tblData1 = {0x01, 0x80, 0x43, 0x00, 0x25, 0x4C, 0xE9, 0x00, 0xDB, 0x8F, 0x16, 0x01, 0x01, 0x04, 0x01, 0x01, 0x02, 0x06, 0x00, 0x00, 0x00, 0x04, 0x00, 0x03, 0x00, 0x06, 0x00, 0x08, 0x00, 0x05, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x03, 0x00, 0x06, 0x00, 0x08, 0x00, 0x05, 0x03}
	LOGTBL(tblData1)
	LOGTBL(whatWrite(tblData1))

	-- local tblData2 = {0x01, 0x02, 0x10, 0x47, 0x02, 0x10, 0x02, 0x1C, 0x18, 0x02, 0x15, 0x7D, 0xC0, 0x6F, 0xA0, 0x02, 0x10, 0xE4, 0x4C, 0xE3, 0x6F, 0x02, 0x10, 0x02, 0x10, 0x03}
	-- LOGTBL(tblData2)
	-- LOGTBL(whatRead(tblData2))

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
	print("total is "..#tblData.."\r\n")
	while where < #tblData do
		local tmpLen = (tblData[where + 3] << 8) | tblData[where + 4]
		local tmpString = string.sub(data, where, (where + 7 + tmpLen - 1))
		local tmpTbl = stringToTable(tmpString)
		if tmpTbl[6] ~= csum(tmpTbl) then
			break
		end
		LOGTBL(tmpTbl)
		tblDataCountLen[idx + 1] = (7 + tmpLen) & 0xFF
		tblDataCountLen[idx + 2] = 0x00
		idx = idx + 2
		where = where + 7 + tmpLen
	end

	strDataCountLen = tableToString(tblDataCountLen)
	LOGTBL(tblDataCountLen)
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
	local WHATKIND_SUB_DEV_INFO = 14

	local tmp = stringToTable(data)
	tmp = whatRead(tmp)
	LOGTBL(tmp)
	data = tableToString(tmp)

	-- test only
	cvtType = 0x01

	for i = 1, 1 do
		-- UART
		if 0x01 == cvtType then
			dataTbl = stringToTable(data)
			-- START for status of sub devices
			if nil ~= string.find(data, string.char(0x01, 0x00, 0x4D, 0x00, 0x0B)) then
				-- (IND RAW) new device joining, RAW ind is 0102104D02100B2331BC60AF76337EE10219748003
				print ("[LUA] s1GetValidKind - new device joining\r\n")
				ret = WHATKIND_SUB_DEV_JOIN
				break
			end

			if nil ~= string.find(data, string.char(0x01, 0x80, 0x48, 0x00, 0x09)) then
				-- (IND RAW) new device leaving, RAW ind is 01804802100219A960AF76337EE1021974021003 
				print ("[LUA] s1GetValidKind - new device leaving\r\n")
				ret = WHATKIND_SUB_DEV_LEAVE
				break
			end

			if nil ~= string.find(data, string.char(0x01, 0x80, 0x45)) or 
				nil ~= string.find(data, string.char(0x01, 0x80, 0x43)) then
				-- (RSP RAW) RAW endpoint list 018045021002167fe80210db8f0211021103 
				-- (RSP RAW) RAW endpoint info 0180430210254ce90210db8f160211021102140211021102120216021002100210021402100213021002160210021802100215021102100210021002140210021302100216021002180210021503 
				print ("[LUA] s1GetValidKind - ENDPOINT list or info "..#dataTbl.."\r\n")
				ret = WHATKIND_SUB_DEV_INFO
				break
			end

			if nil ~= string.find(data, string.char(0x01, 0x81, 0x02)) then
				-- (IND RAW) sDevStatus, RAW ind 0181021202100B2D0212853002150210021602100210021010021103 
				print ("[LUA] s1GetValidKind - sDevStatus ind "..#dataTbl.."\r\n")
				ret = WHATKIND_SUB_DEV_DATA
				break
			end

			-- TODO: if it is really need
			if nil ~= string.find(data, string.char(0x01, 0x80, 0x06, 0x00, 0x01, 0x86, 0x01, 0x03)) or
				nil ~= string.find(data, string.char(0X01, 0X80, 0X00, 0X00, 0X04)) then
				-- (RSP RAW) reset FAC rsp , RAW rep is 018002160210021186021103 
				-- (RSP RAW) join permition rsp , RAW rep is 0180021002100214380210F502104903 
				print ("[LUA] s1GetValidKind - RSP(s) "..#dataTbl.."\r\n")
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
-- \{\"ctrl\":\{\"sDevLeave\":\{\"addr\":\"057d\",\"mac\":\"C06FA000E44CE36F\"\}\}\}
-- \{\"ctrl\":\{\"sDevGetList\":1\}\}
-- \{\"ctrl\":\{\"sDevGetInfo\":0\}\}
-- \{\"ctrl\":\{\"sDevGetInfo\":1\}\}
-- \{\"ctrl\":\{\"pwr\":1,\"sDev\":\{\"pid\":\"0104\",\"did\":\"0107\",\"clu\":\"0006\",\"ept\":[1,2],\"mac\":\"7409E17E3376AF60\"\}\}\}
local function bin2hex(s)
    s=string.gsub(s,"(.)",function (x) return string.format("%02X",string.byte(x)) end)
    return s
end

function hex2bin(str)
	local tmp = ''
	for i = 1, string.len(str), 2 do 
		local sub = string.sub(str, i, i+1)
		local h = string.byte(sub, 1)
		local l = string.byte(sub, 2)
		if h >= 0x30 and h <= 0x39 then
			h = (h - 0x30) * 16
		elseif h >= 0x41 and h <= 0x46 then
			h = (h - 0x41 + 10) * 16
		elseif h >= 0x61 and h <= 0x66 then
			h = (h - 0x61 + 10) * 16
		end
		if l >= 0x30 and l <= 0x39 then
			l = l - 0x30
		elseif l >= 0x41 and l <= 0x46 then
			l = l - 0x41 + 10
		elseif l >= 0x61 and l <= 0x66 then
			l = l - 0x61 + 10
		end
		tmp = tmp .. string.char(h + l)
		end
	return tmp
end


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

	-- test only
	cvtType = 1

	for i = 1, 1 do
		-- UART
		if 0x01 == cvtType then
			-- RAW is  01 02 10 11 02 10 02 10 11 03
			if ctrl["reset"] == 1 then
				cmdTbl = {0x01, 0x00, 0x11, 0x00, 0x00, 0x11, 0x03}
				break
			end

			if ctrl["sDevJoin"] == 1 then
			-- RAW is  01 02 10 49 02 10 02 14 7E FF FC 30 02 10 03
				cmdTbl = {0x01, 0x00, 0x49, 0x00, 0x04, 0x7E, 0xFF, 0xFC, 0x30, 0x00, 0x03}
 				break
			end

			if ctrl["sDevLeave"] then
			-- NXP is 01 00 47 00 0C 18 05 7D C0 6F A0 00 E4 4C E3 6F 00 00 03
			-- RAW is 01 02 10 47 02 10 02 1C 18 02 15 7D C0 6F A0 02 10 E4 4C E3 6F 02 10 02 10 03
				local addr = ctrl["sDevLeave"]["addr"]
				local mac = ctrl["sDevLeave"]["mac"]
				local str = string.char(0x01, 0x00, 0x47, 0x00, 0x0C, 0x00)
				str = str .. hex2bin(addr) .. hex2bin(mac) .. string.char(0x00, 0x00, 0x03)
				cmdTbl = stringToTable(str)
				cmdTbl[6] = csum(cmdTbl)
				-- LOGTBL(cmdTbl)
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

	cmdTbl = whatWrite(cmdTbl)
	-- LOGTBL(cmdTbl)
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
	-- local strSubDev = '"sDev":{"pid":"%s","clu":"%s","ept":%s,"mac":"%s"}'
	local strSubDev = '"sDev":[%s],"sMac":%s'
	dataTbl = stringToTable(bin)

	dataTbl = whatRead(dataTbl)
	LOGTBL(dataTbl)
	bin = tableToString(dataTbl)

	-- test only
	cvtType = 0x01

	for i = 1, 1 do
		-- UART
		if 0x01 == cvtType then
			-- INTERNAL
			-- (RSP) ept list {"idx":"DB8F","ept":[1,2]}
			if nil ~= string.find(bin, string.char(0x01, 0x80, 0x45)) then
				hex2bin(string.sub(bin,7,8))
				strSubDev = '{"idx":"'..bin2hex(string.sub(bin,9,10))..'","ept":['
				for i = 12, dataTbl[11]+11 do
					strSubDev = strSubDev..dataTbl[i]..','
				end
				strSubDev = string.sub(strSubDev,1,string.len(strSubDev) - 1)..']}'
				print("result -> "..strSubDev..'\r\n')
				break
			end
			
			-- (RSP) ept list {"idx":"DB8F","ept":1,"did":"0101","clu":["0000","0004"]}
			if nil ~= string.find(bin, string.char(0x01, 0x80, 0x43)) then
				break
			end
			-- EXTERNAL
			-- (RSP) join, leave
			-- (IND) ind actions
			-- if nil ~= string.find(bin, string.char(0x01, 0x80, 0x00, 0x00, 0x04)) then
			-- 	-- (RSP) join permition rsp , RAW rsp is 0180021002100214380210F502104903  
			-- 	print ("[LUA] s1CvtPri2Std - RSP - join permition or mgnt leave rsp\r\n")
			-- 	strMain = string.format('{"sDevActRsp":%d}', 2)
			-- 	break
			-- end

			if nil ~= string.find(bin, string.char(0x01, 0x00, 0x4D)) then
				-- (IND) new device joining, RAW rep is 0102104d0210021b5abe4a60af76337ee10219748003 
				-- {"sDevJoin":2,"sDev":{"pid":"0401","clu":"0107","ept":[["0701",1]],"mac":"6FE34CE400A06FC0"}}
				-- {"sDevJoin":2,"sDev":{"pid":"0401","ept":[["0701",0],["0701",1]],"mac":"6FE34CE400A06FC0"}}
				bin2hex(string.sub(bin, 7, 8))
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

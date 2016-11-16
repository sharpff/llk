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

	-- local tblData1 = {0x01, 0x80, 0x00, 0x00, 0x04, 0x99, 0x00, 0x54, 0x00, 0x49, 0x03}
	-- LOGTBL(tblData1)
	-- LOGTBL(whatWrite(tblData1))

	local tblData2 = {0x01, 0x80, 0x43, 0x00, 0x25, 0x00, 0xe9, 0x00, 0xdb, 0x8f, 0x16, 0x01, 0x01, 0x04, 0x04, 0x02, 0x02, 0x06, 0x00, 0x00, 0x00, 0x04, 0x00, 0x03, 0x00, 0x06, 0x00, 0x08, 0x00, 0x05, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x03, 0x00, 0x06, 0x00, 0x08, 0x00, 0x05, 0x03}
	-- LOGTBL(tblData2)
	tblData2[6] = csum(tblData2)
	LOGTBL(whatWrite(tblData2))
	-- s1apiOptLogTable(tblData2)
	-- local a = tableToString(tblData2)
	-- local a = s1apiOptTable2String(tblData2)
	print('abc'..'\r\n')
	-- LOGTBL(whatRead(tblData2))


	-- local str = s1apiOptTable2String(tblData2)
	-- s1apiOptLogTable(s1apiOptString2Table(string.len(str), str))
	-- local tblData3 = s1apiOptString2Table(string.len(str), str)
	-- s1apiOptLogTable(tblData3)
	-- local str = s1apiOptTable2String(tblData2)
	-- s1apiOptLogTable(s1apiOptString2Table(string.len(str), str))
	-- local tblData3 = s1apiOptString2Table(string.len(str), str)
	-- s1apiOptLogTable(tblData3)
	-- local str = s1apiOptTable2String(tblData2)
	-- s1apiOptLogTable(s1apiOptString2Table(string.len(str), str))
	-- local tblData3 = s1apiOptString2Table(string.len(str), str)
	-- s1apiOptLogTable(tblData3)
	-- local str = s1apiOptTable2String(tblData2)
	-- s1apiOptLogTable(s1apiOptString2Table(string.len(str), str))
	-- local tblData3 = s1apiOptString2Table(string.len(str), str)
	-- s1apiOptLogTable(tblData3)
	-- local str = s1apiOptTable2String(tblData2)
	-- s1apiOptLogTable(s1apiOptString2Table(string.len(str), str))
	-- local tblData3 = s1apiOptString2Table(string.len(str), str)
	-- s1apiOptLogTable(tblData3)
	-- local str = s1apiOptTable2String(tblData2)
	-- s1apiOptLogTable(s1apiOptString2Table(string.len(str), str))
	-- local tblData3 = s1apiOptString2Table(string.len(str), str)
	-- s1apiOptLogTable(tblData3)
	local str = '1.0'
	return string.len(str), str
end

--[[ EXTERNAL
	s1GetCvtType
  ]]
function s1GetCvtType()
    -- combained uart(0x1) & pwm(0x10) & pwm(0x20)
    local str = [[
    {"whatCvtType":49,
     "common":[{"num":7,"id":"36-37-32-33-34-35-0","mux":"7-7-9-9-9-9-3"}],
     "uart":[{"id":1, "baud":"115200-8N1"}],  
     "eint":[{"id":0,"gid":0,"type":2,"mode":4,"state":1,"debounce":5,"timeout":400}],
     "pwm":[{"id":33,"type":1,"clock":1,"state":0,"frequency":5120,"duty":1024},
            {"id":34,"type":1,"clock":1,"state":0,"frequency":5120,"duty":1024},
            {"id":35,"type":1,"clock":1,"state":0,"frequency":5120,"duty":1024},
            {"id":18,"type":1,"clock":1,"state":0,"frequency":5120,"duty":1024}]
    }
    ]]
	local delay = 5
	return string.len(str), str, delay
end

--[[ EXTERNAL
	s1GetQueries
  ]]
function s1GetQueries(queryType)
	return 0, "", 0, ""
end

function s1OptHasSubDevs()
	return 1
end

--[[ OPTIONAL
	s1OptDoSplit
  ]]
function s1OptDoSplit(data)
	local cvtType = s1apiGetCurrCvtType()
	local tblDataCountLen = {}
	local strDataCountLen = ""
	local where = 1
	local idx = 0
	local tblData = stringToTable(data)
	if 0x01 == cvtType then
		tblData = whatRead(tblData)
		data = tableToString(tblData)
		print("[LUA] total ======> "..#tblData.."\r\n")
		-- LOGTBL(tblData)
		-- print("[LUA] <============ \r\n")

		while where < #tblData do
			local tmpLen = (tblData[where + 3] << 8) | tblData[where + 4]
			-- print("tmpLen is "..tmpLen.."\r\n")
			local tmpString = string.sub(data, where, (where + 7 + tmpLen - 1))
			local tmpTbl = stringToTable(tmpString)
			if tblData[where+6-1] ~= csum(tmpTbl) then
				print("[LUA E] csum failed\r\n")
				break
			end
			tblDataCountLen[idx + 1] = #(whatWrite(tmpTbl)) & 0xFF
			tblDataCountLen[idx + 2] = 0x00
			idx = idx + 2
			where = where + 7 + tmpLen
			-- print("[LUA] IDX @"..where.." & LEN is "..tmpLen.." ------->\r\n")
			-- LOGTBL(tmpTbl)
			-- print("[LUA] --------------------------------- \r\n")
		end

		-- print("[LUA] out ========> \r\n")
		-- LOGTBL(tblDataCountLen)
		print("[LUA] <============ \r\n")
		-- print(string.format('count [%d] ', string.len( strDataCountLen)) .. LOGTBL(stringToTable(strDataCountLen)))
	else
		tblDataCountLen[1] = (#tblData) & 0xFF
		tblDataCountLen[2] = 0x00
	end
	strDataCountLen = tableToString(tblDataCountLen)
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

    -- included eint(0x20) & pwm(0x10)
	if 0x10 == cvtType or 0x20 == cvtType then
		return WHATKIND_MAIN_DEV_DATA;
	end

	local tmp = stringToTable(data)
	tmp = whatRead(tmp)
	print("[LUA] s1GetValidKind start\r\n")
	LOGTBL(tmp)
	data = tableToString(tmp)

	-- test only
	-- cvtType = 0x01

	for i = 1, 1 do
		-- UART
		if 0x01 == cvtType then
			dataTbl = stringToTable(data)
			-- START for status of sub devices
			if nil ~= string.find(data, string.char(0x01, 0x00, 0x4D, 0x00, 0x0B)) then
				-- (IND) new device joining, RAW ind is 0102104D02100B2331BC60AF76337EE10219748003
				print ("[LUA] s1GetValidKind - new device joining\r\n")
				ret = WHATKIND_SUB_DEV_JOIN
				break
			end

			if nil ~= string.find(data, string.char(0x01, 0x80, 0x48, 0x00, 0x09)) then
				-- (IND) new device leaving, RAW ind is 01804802100219A960AF76337EE1021974021003 
				print ("[LUA] s1GetValidKind - new device leaving\r\n")
				ret = WHATKIND_SUB_DEV_LEAVE
				break
			end

			if nil ~= string.find(data, string.char(0x01, 0x80, 0x45)) or 
				nil ~= string.find(data, string.char(0x01, 0x80, 0x43)) or
				nil ~= string.find(data, string.char(0x01, 0x80, 0x42)) then
				-- (RSP) query ept, query ept info, query man
				print ("[LUA] s1GetValidKind - ENDPOINT list or info "..#dataTbl.."\r\n")
				ret = WHATKIND_SUB_DEV_INFO
				break
			end

			if nil ~= string.find(data, string.char(0x01, 0x81, 0x02)) or
				nil ~= string.find(data, string.char(0x01, 0x84, 0x01)) then
				-- (IND) sDevStatus, RAW ind 0181021202100B2D0212853002150210021602100210021010021103 
				print ("[LUA] s1GetValidKind - sDevStatus ind "..#dataTbl.."\r\n")
				ret = WHATKIND_SUB_DEV_DATA
				break
			end

			-- TODO: if it is really need
			if nil ~= string.find(data, string.char(0x01, 0x80, 0x06, 0x00, 0x01, 0x86, 0x01, 0x03)) or
				nil ~= string.find(data, string.char(0X01, 0X80, 0X00, 0X00, 0X04)) then
				-- (RSP) reset FAC rsp , RAW rep is 018002160210021186021103 
				-- (RSP) join permition rsp , RAW rep is 0180021002100214380210F502104903 
				print ("[LUA] s1GetValidKind - RSP(s) "..#dataTbl.."\r\n")
				ret = WHATKIND_SUB_DEV_DATA
				break
			end

		end

	end
	-- invalid kind
	print ("[LUA] s1GetValidKind - ret is "..ret.."\r\n")

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

function genStatus(clu, ept, val)
	local status = "{}"
	print("clu is "..clu..'\r\n')
	print("ept is "..ept..'\r\n')
	print("val is "..val..'\r\n')

	-- print("did "..dept[1]..", ept "..dept[2].."\r\n")
	if nil ~= string.find(clu, "0006") then 
		status = string.format('{"switcher":%d}', val)
	elseif nil ~= string.find(clu, "0406") then
		status = string.format('{"pir":%d}', val)
	elseif nil ~= string.find(clu, "0500") then
		status = string.format('{"ias":%d}', val)
	end
	return status
end

function genSDevCtrl(tblSDev, tblSDevCtrl)
	local cmd = ''
	-- for light cluster
	if tblSDev["ept"] == 1 and tblSDev["did"] == "0101" and tblSDev["clu"] == "0006" then
		if tblSDevCtrl["pwr"] == 1 then
			cmd = string.char(0x01)
		elseif tblSDevCtrl["pwr"] == 0 then 
			cmd = string.char(0x00)
		else
			cmd = string.char(0x02)
		end	
	end
	return cmd
end

--[[ EXTERNAL
	s1CvtStd2Pri
  ]]
function s1CvtStd2Pri(json)
	local cvtType = s1apiGetCurrCvtType()
	print ('[LUA] s1CvtStd2Pri return => '..json..'\r\n')
	local ctrl = cjson.decode(json)
	local cmdTbl = {}
	local dataStr = ""

	-- test only
	-- cvtType = 1

		-- UART
		if 0x01 == cvtType then
			for x = 1, 1 do
				local sDevCtrl = ctrl["sDevCtrl"]
				-- RAW is  01 02 10 11 02 10 02 10 11 03
				if ctrl["reset"] == 1 then
					cmdTbl = {0x01, 0x00, 0x11, 0x00, 0x00, 0x11, 0x03}
					break
				end

				if ctrl["sDevJoin"] then
					cmdTbl = {0x01, 0x00, 0x49, 0x00, 0x04, 0x00, 0xFF, 0xFC, 0x00, 0x00, 0x03}
					cmdTbl[9] = ctrl["sDevJoin"]
					cmdTbl[6] = csum(cmdTbl)
					-- RAW is  01 02 10 49 02 10 02 14 7E FF FC 30 02 10 03
					-- cmdTbl = {0x01, 0x00, 0x49, 0x00, 0x04, 0x7E, 0xFF, 0xFC, 0x30, 0x00, 0x03}
					-- cmdTbl = {0x01, 0x00, 0x49, 0x00, 0x04, 0x46, 0xff, 0xfc, 0x08, 0x00, 0x03}
					-- cmdTbl = {0x01, 0x00, 0x49, 0x00, 0x04, 0xb1, 0xff, 0xfc, 0xff, 0x00, 0x03}
					-- cmdTbl = {0x01, 0x00, 0x49, 0x00, 0x04, 0x4e, 0xff, 0xfc, 0x00, 0x00, 0x03}
	 				break
				end

				-- "{\"sDevLeave\":1,\"idx\":\"ABCD\",\"mac\":\"C06FA000E44CE36F\"}"
				if ctrl["sDevLeave"] then
				-- NXP is 01 00 47 00 0C 18 05 7D C0 6F A0 00 E4 4C E3 6F 00 00 03
				-- RAW is 01 02 10 47 02 10 02 1C 18 02 15 7D C0 6F A0 02 10 E4 4C E3 6F 02 10 02 10 03
					local addr = ctrl["idx"]
					local mac = ctrl["mac"]
					local str = string.char(0x01, 0x00, 0x47, 0x00, 0x0C, 0x00)
					str = str .. hex2bin(addr) .. hex2bin(mac) .. string.char(0x00, 0x00, 0x03)
					cmdTbl = stringToTable(str)
					cmdTbl[6] = csum(cmdTbl)
					LOGTBL(cmdTbl)
	 				break
				end

				-- INTERNAL QUERY
				-- "{\"sDevQryEpt\":1,\"idx\":\"ABCD\"}"
				if ctrl["sDevQryEpt"] == 1 then
					local str = string.char(0x01, 0x00, 0x45, 0x00, 0x02, 0x00)
					local addr = ctrl["idx"]
					str = str..hex2bin(addr)..string.char(0x03)
					cmdTbl = stringToTable(str)
					cmdTbl[6] = csum(cmdTbl)
					LOGTBL(cmdTbl)
					break
				end
				-- "{\"sDevQryMan\":1,\"idx\":\"ABCD\"}"
				if ctrl["sDevQryMan"] == 1 then
					local str = string.char(0x01, 0x00, 0x42, 0x00, 0x02, 0x00)
					local addr = ctrl["idx"]
					str = str..hex2bin(addr)..string.char(0x03)
					cmdTbl = stringToTable(str)
					cmdTbl[6] = csum(cmdTbl)
					LOGTBL(cmdTbl)
					break
				end
				-- "{\"sDevQryEptInfo\":1,\"idx\":\"ABCD\",\"ept\":1}"
				if ctrl["sDevQryEptInfo"] == 1 then
					local str = string.char(0x01, 0x00, 0x43, 0x00, 0x03, 0x00)
					local addr = ctrl["idx"]
					local ept = ctrl["ept"]
					str = str..hex2bin(addr)..string.char(ept, 0x03)
					cmdTbl = stringToTable(str)
					cmdTbl[6] = csum(cmdTbl)
					LOGTBL(cmdTbl)
					break
				end

				-- TODO: ctrl the sub dev.
				if sDevCtrl then
					local str = string.char(0x01, 0x00, 0x92, 0x00, 0x00, 0x00, 0x02)
					local mac = ctrl["sDev"]["mac"]
					local dEpt = ctrl["sDev"]["ept"]
					-- local idx = s1apiSdevGetUserDataByMac(mac)
					local idx = string.char(0x31, 0x71)
					str = str..idx..string.char(0x01)..string.char(dEpt)..genSDevCtrl(ctrl["sDev"], sDevCtrl)..string.char(0x03)
					cmdTbl = stringToTable(str)
					-- len
					cmdTbl[5] = (string.len(str) - 7) & 0xff
					cmdTbl[4] = ((string.len(str) - 7) >> 8) & 0xff
					cmdTbl[6] = csum(cmdTbl)
					LOGTBL(cmdTbl)
					break
				end

				if itself then
					print("TODO: ctrl the self dev. \r\n")
					break
				end
			end
			cmdTbl = whatWrite(cmdTbl)
			print("whatWrite :")
			LOGTBL(cmdTbl)
			-- LOGTBL(cmdTbl)
		elseif cvtType == 0x10 then
			local i = 0, val
			local j = 0
			local aa = ctrl["pwm"]
			if aa then
				for j = 1, 4 do
					val = aa[j]["id"]
					i = i + 1
					cmdTbl[i] = val
					val = aa[j]["val"]
					i = i + 1
					cmdTbl[i] = (val >> 8) & 0xff
					i = i + 1
					cmdTbl[i] = val & 0xFF
				end
			end
		end

	dataStr = tableToString(cmdTbl)
	return string.len(dataStr), dataStr
end

--[[ EXTERNAL
	s1CvtPri2Std
  ]]
function s1CvtPri2Std(bin)
	local cvtType = s1apiGetCurrCvtType()
	local dataTbl = {}
	local str = ''
    local len = 0
	local status = '{"pwm":[{"id":%d,"val":%d},{"id":%d,"val":%d},{"id":%d,"val":%d},{"id":%d,"val":%d}],"eint":[{"id":%d,"val":%d}]}'
    local PRI2STD_LEN_NONE = 0x00000000
    local PRI2STD_LEN_INTERNAL = 0x40000000
    local PRI2STD_LEN_BOTH = 0x20000000
    local PRI2STD_LEN_MAX = 0x0000FFFF

	-- test only
	-- cvtType = 0x01

	-- UART
	if 0x01 == cvtType then
		for x = 1, 1 do
			str = '{}'
			-- INTERNAL
			-- local str = '"sDev":{"pid":"%s","clu":"%s","ept":%s,"mac":"%s"}'
			dataTbl = stringToTable(bin)
			LOGTBL(dataTbl)

			dataTbl = whatRead(dataTbl)
			print("s1CvtPri2Std\r\n")
			LOGTBL(dataTbl)
			bin = tableToString(dataTbl)
			-- (RSP) ept list {"sDevQryEpt":2,"idx":"DB8F","ept":[1,2]}
			if nil ~= string.find(bin, string.char(0x01, 0x80, 0x45)) then
				str = '{"sDevQryEpt":2,"idx":"'..bin2hex(string.sub(bin,9,10))..'","ept":['
				for i = 12, dataTbl[11]+11 do
					str = str..dataTbl[i]..','
				end
				str = string.sub(str,1,string.len(str) - 1)..']}'
				break
			end
			
			-- (RSP) ept info {"sDevQryEptInfo":2,"idx":"DB8F","pid":"0104","ept":1,"did":"0101","clu":["0000","0004"]}
			if nil ~= string.find(bin, string.char(0x01, 0x80, 0x43)) then
				str = '{"sDevQryEptInfo":2,"idx":"'..bin2hex(string.sub(bin,9,10))..'","pid":"'..bin2hex(string.sub(bin,13,14))..'","ept":'..dataTbl[12]..',"did":"'..bin2hex(string.sub(bin,15,16))..'"'
				local m = 18
				if dataTbl[m] > 0 then
					str = str..',"cluI":['
					for i = m+1, dataTbl[m]*2+m, 2 do
						str = str..'"'..bin2hex(string.sub(bin,i,i+1))..'",'
					end
					str = string.sub(str,1,string.len(str) - 1)..']'
				end
				m = 18+dataTbl[18]*2+1
				if dataTbl[m] > 0 then
					str = str..',"cluO":['
					for i = m+1, dataTbl[m]*2+m, 2 do
						str = str..'"'..bin2hex(string.sub(bin,i,i+1))..'",'
					end
					str = string.sub(str,1,string.len(str) - 1)..']'
				end
				str = str..'}'
				if nil ~= string.find(string.char(0x04, 0x02), string.sub(bin,15,16)) then
					len = PRI2STD_LEN_BOTH
				end
				break
			end

			-- (RSP) manufacture {"sDevQryMan":2,"idx":"DB8F","man":"1234"}
			if nil ~= string.find(bin, string.char(0x01, 0x80, 0x42)) then
				str = '{"sDevQryMan":2,"idx":"'..bin2hex(string.sub(bin,9,10))..'","man":"'..bin2hex(string.sub(bin,11,12))..'"}'
				break
			end

			-- INTERNAL -> EXTERNAL
			-- (IND) join, leave
			if nil ~= string.find(bin, string.char(0x01, 0x00, 0x4D)) then
				-- {"sDevJoin":2,"sDev":{"idx":"DB8F","mac":"6FE34CE400A06FC0"}}
				str = '{"sDevJoin":2,"sDev":{"idx":'..'"'..bin2hex(string.sub(bin,7,8))..'"'..',"mac":'..'"'..bin2hex(string.sub(bin,9,16))..'"'..'}}'
				-- str = '{"sDevJoin":2,"sDev":{"idx":"DB8F","mac":'..'"'..bin2hex(string.sub(bin,9,16))..'"'..'}}'
				break
			end
			if nil ~= string.find(bin, string.char(0x01, 0x80, 0x48)) then
				-- {"sDevLeave":2,"sDev":{mac":"6FE34CE400A06FC0"}}
				str = '{"sDevLeave":2,"sDev":{"mac":'..'"'..bin2hex(string.sub(bin,7,14))..'"'..'}}'
				break
			end
			-- (IND) ind actions
			-- {"sDevStatus":{"btn":1},"sDev":{"idx":"DB8F"}}
			if nil ~= string.find(bin, string.char(0x01, 0x81, 0x02)) then
				str = '{"sDevStatus":'..genStatus(bin2hex(string.sub(bin,11,12)), bin2hex(string.sub(bin,10,10)), bin2hex(string.sub(bin,17,17)))..',"sDev":{"idx":"'..bin2hex(string.sub(bin,8,9)).. '"}}'
				break
			end

			-- (IAS IND) 018401000da50401050002c5eb00000000000103 
			if nil ~= string.find(bin, string.char(0x01, 0x84, 0x01)) then
				str = '{"sDevStatus":'..genStatus(bin2hex(string.sub(bin,9,10)), bin2hex(string.sub(bin,8,8)), bin2hex(string.sub(bin,14,15)))..',"sDev":{"idx":"'..bin2hex(string.sub(bin,12,13)).. '"}}'
				break
			end
		end
		print("result -> "..str..'\r\n')
	elseif 0x20 == cvtType then
		dataTbl = {33, 0, 34, 0, 35, 0, 18, 0}
		local lenStatus, currStatus = s1apiGetDevStatus()
		if lenStatus <= 2 then
			str = string.format(status, 33, 0, 34, 0, 35, 0, 18, 0, 0, 0)
		end
		local id = bin:byte(1)
		local i = 0, val
		if id == 0 then
            if lenStatus > 2 then
                local tb = cjson.decode(currStatus)
                local pwm = tb["pwm"]
                for j = 1, 4 do
                    val = pwm[j]["id"]
                    i = i + 1
                    dataTbl[i] = val
                    val = pwm[j]["val"]
                    i = i + 1
                    dataTbl[i] = val
                end
            end
            if bin:byte(2) == 1 then
                if dataTbl[8] == 0 then
                    str = string.format(status, dataTbl[1], 1024, dataTbl[3], 1024, dataTbl[5], 1024, dataTbl[7], 1024, 0, 1)
                else
                    str = string.format(status, dataTbl[1], 0, dataTbl[3], 0, dataTbl[5], 0, dataTbl[7], 0, 0, 1)
                end
            elseif bin:byte(2) == 2 then
                str = string.format(status, dataTbl[1], 0x8001, dataTbl[3], 0x8001, dataTbl[5], 0x8001, dataTbl[7], 1024, 0, 2)
            elseif bin:byte(2) == 3 then
                str = string.format(status, dataTbl[1], 0x8002, dataTbl[3], 0x8002, dataTbl[5], 0x8002, dataTbl[7], 1024, 0, 3)
            elseif bin:byte(2) == 0xFF then
                str = string.format(status, dataTbl[1], 0x8003, dataTbl[3], 0x8003, dataTbl[5], 0x8003, dataTbl[7], 1024, 0, 0xFF)
            elseif bin:byte(2) == 0 then
                str = string.format(status, dataTbl[1], dataTbl[2], dataTbl[3], dataTbl[4], dataTbl[5], dataTbl[6], dataTbl[7], dataTbl[8], 0, 0)
            end
        end
		len = PRI2STD_LEN_BOTH
	elseif 0x10 == cvtType then
		if #bin >= 12 then
			local id1, id2, id3, id4, val1, val2, val3, val4
			local id5 = 0
            local val5 = 0
            local lenStatus1, currStatus1 = s1apiGetDevStatus()
            if lenStatus1 > 2 then
	            local tb1 = cjson.decode(currStatus1)
	            local eintData = tb1["eint"]
	            id5 = eintData[1]["id"]
	            val5 = eintData[1]["val"]
        	end
			id1 = bin:byte(1)
			val1 = bin:byte(2)
			val1 = (val1 << 8) | bin:byte(3)
			id2 = bin:byte(4)
			val2 = bin:byte(5)
			val2 = (val2 << 8) | bin:byte(6)      
			id3 = bin:byte(7)
			val3 = bin:byte(8)
			val3 = (val3 << 8) | bin:byte(9)         
			id4 = bin:byte(10)
			val4 = bin:byte(11)
			val4 = (val4 << 8) | bin:byte(12) 
			str = string.format(status, id1, val1, id2, val2, id3, val3, id4, val4, id5, val5)	
		end
	end
	return string.len(str) + len, str
end

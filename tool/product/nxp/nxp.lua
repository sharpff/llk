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

	-- local tblData1 = {0x01, 0x00, 0x11, 0x00, 0x00, 0x11, 0x03}
	-- LOGTBL(tblData1)
	-- LOGTBL(whatWrite(tblData1))

	-- local tblData2 = {0x01, 0x80, 0x43, 0x00, 0x25, 0x00, 0xe9, 0x00, 0xdb, 0x8f, 0x16, 0x01, 0x01, 0x04, 0x04, 0x02, 0x02, 0x06, 0x00, 0x00, 0x00, 0x04, 0x00, 0x03, 0x00, 0x06, 0x00, 0x08, 0x00, 0x05, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x03, 0x00, 0x06, 0x00, 0x08, 0x00, 0x05, 0x03}
	-- LOGTBL(tblData2)
	-- tblData2[6] = csum(tblData2)
	-- LOGTBL(whatWrite(tblData2))
	-- s1apiOptLogTable(tblData2)
	-- local a = tableToString(tblData2)
	-- local a = s1apiOptTable2String(tblData2)
	-- print('abc'..'\r\n')
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
	local str = '1.0.1'
	return string.len(str), str
end

--[[ EXTERNAL
	s1GetCvtType
  ]]
function s1GetCvtType()
    -- combained uart(0x1) & pwm(0x10) & eint(0x20)
    local str = [[
    {"whatCvtType":32801,
     "common":[{"num":3,"id":"36-37-0","mux":"7-7-3"}],
     "uart":[{"id":1, "baud":"115200-8N1"}],  
     "eint":[{"id":0,"gid":0,"type":1,"mode":3,"trigger":4,"state":1,"debounce":5,"timeout":400}]
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
	local ud = 0
	local tblData = stringToTable(data)
	local dataLen = 0
	if 0x01 == cvtType then
		tblData = whatRead(tblData)
		data = tableToString(tblData)
		dataLen = #tblData
		-- if #tblData > 96 then
		-- 	dataLen = 96
		-- end
		print("[LUA] total ======> "..dataLen.."\r\n")
		-- LOGTBL(tblData)
		-- print("[LUA] <============ \r\n")

		while where < dataLen do
			local tmpLen = (tblData[where + 3] << 8) | tblData[where + 4]
			local tmpString = string.sub(data, where, (where + 7 + tmpLen - 1))
			local tmpTbl = stringToTable(tmpString)
			if tblData[where+6-1] ~= csum(tmpTbl) then
				print("[LUA E] csum failed\r\n")
				break
			end
			tblDataCountLen[ud + 1] = #(whatWrite(tmpTbl)) & 0xFF
			tblDataCountLen[ud + 2] = 0x00
			ud = ud + 2
			where = where + 7 + tmpLen
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

    -- included eint(0x20) & user type(0x8000)
	if 0x20 == cvtType or 0x8000 == cvtType then
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

			if nil ~= string.find(data, string.char(0x01, 0x80, 0x10)) then
				ret = WHATKIND_SUB_DEV_DATA
				break;
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
-- \{\"ctrl\":\{\"sDevDel\":1,\"mac\":\"C06FA000E44CE36F\"\}\}
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
	print ('[LUA] s1CvtStd2Pri type['..cvtType..'] => '..json..'\r\n')
	local ctrl = cjson.decode(json)
	local cmdTbl = {}
	local dataStr = ""

	-- test only
	-- local cvtType = 1

		-- UART
		if 0x01 == cvtType then
			for x = 1, 1 do
				local sDevCtrl = ctrl["sDevCtrl"]
				if ctrl["sDevChnl"] then
					local a = 0x00000800 << (ctrl["sDevChnl"] - 11)
					cmdTbl = {0x01, 0x00, 0x21, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03}
					cmdTbl[10] = 0x00
					cmdTbl[9] = 0xFF & (a >> 8)
					cmdTbl[8] = 0xFF & (a >> 16)
					cmdTbl[7] = 0xFF & (a >> 24)
					cmdTbl[6] = csum(cmdTbl)
					LOGTBL(cmdTbl)
					break
				end

				-- "{"ctrl":{\"sDevReset\":1}}"
				if ctrl["sDevReset"] == 1 then
					cmdTbl = {0x01, 0x00, 0x11, 0x00, 0x00, 0x11, 0x03}
					break
				end

				-- "{"ctrl":{\"sDevVer\":1}}"
				if ctrl["sDevVer"] == 1 then
					cmdTbl = {0x01, 0x00, 0x10, 0x00, 0x00, 0x10, 0x03}
					break
				end

				-- "{"ctrl":{\"sDevJoin\":45}}"
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

				-- "{"ctrl":{\"sDevDel\":1,\"mac\":\"C06FA000E44CE36F\"}}"
				if ctrl["sDevDel"] then
				-- NXP is 01 00 47 00 0C 18 05 7D C0 6F A0 00 E4 4C E3 6F 00 00 03
				-- RAW is 01 02 10 47 02 10 02 1C 18 02 15 7D C0 6F A0 02 10 E4 4C E3 6F 02 10 02 10 03
					local mac = ctrl["mac"]
					-- local addr = "1234"
					local ud = s1apiSdevGetUserDataByMac(mac)
					local str = string.char(0x01, 0x00, 0x47, 0x00, 0x0C, 0x00)
					str = str .. hex2bin(ud) .. hex2bin(mac) .. string.char(0x00, 0x00, 0x03)
					cmdTbl = stringToTable(str)
					cmdTbl[6] = csum(cmdTbl)
					LOGTBL(cmdTbl)
	 				break
				end

				-- "{"ctrl":{\"sDevClr\":1}}"
				if ctrl["sDevClr"] then
					-- NXP is 01 00 12 00 00 12 03 
					-- RAW is 01 02 10 12 02 10 02 10 12 03 
					local str = string.char(0x01, 0x00, 0x12, 0x00, 0x00, 0x12, 0x03)
					cmdTbl = stringToTable(str)
					LOGTBL(cmdTbl)
	 				break
				end

				-- INTERNAL QUERY
				-- "{\"sDevQryEpt\":1,\"ud\":\"ABCD\"}"
				if ctrl["sDevQryEpt"] == 1 then
					local str = string.char(0x01, 0x00, 0x45, 0x00, 0x02, 0x00)
					local addr = ctrl["ud"]
					str = str..hex2bin(addr)..string.char(0x03)
					cmdTbl = stringToTable(str)
					cmdTbl[6] = csum(cmdTbl)
					LOGTBL(cmdTbl)
					break
				end
				-- "{\"sDevQryMan\":1,\"ud\":\"ABCD\"}"
				if ctrl["sDevQryMan"] == 1 then
					local str = string.char(0x01, 0x00, 0x42, 0x00, 0x02, 0x00)
					local addr = ctrl["ud"]
					str = str..hex2bin(addr)..string.char(0x03)
					cmdTbl = stringToTable(str)
					cmdTbl[6] = csum(cmdTbl)
					LOGTBL(cmdTbl)
					break
				end
				-- "{\"sDevQryEptInfo\":1,\"ud\":\"ABCD\",\"ept\":1}"
				if ctrl["sDevQryEptInfo"] == 1 then
					local str = string.char(0x01, 0x00, 0x43, 0x00, 0x03, 0x00)
					local addr = ctrl["ud"]
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
					-- local ud = s1apiSdevGetUserDataByMac(mac)
					local ud = string.char(0x31, 0x71)
					str = str..ud..string.char(0x01)..string.char(dEpt)..genSDevCtrl(ctrl["sDev"], sDevCtrl)..string.char(0x03)
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
			print("uart w:")
			LOGTBL(cmdTbl)
			-- LOGTBL(cmdTbl)
			-- "{\"ctrl\":{\"light\":1,\"mode\":0,\"timeout\":0,\"brightness\":552,\"red\":0,\"green\":0,\"blue\":0,\"service\":0,\"wifimode\":1}}"
		elseif cvtType == 0x8000 then
			local val
			val = ctrl["light"]
	        cmdTbl[1] = val & 0xFF
	        val = ctrl["mode"]
	        cmdTbl[2] = val & 0xFF
	        val = ctrl["timeout"]
	        cmdTbl[3] = val & 0xFF
	        val = ctrl["brightness"]
	        cmdTbl[4] = (val >> 8) & 0xFF
	        cmdTbl[5] = val & 0xFF
	        val = ctrl["red"]
	        cmdTbl[6] = (val >> 8) & 0xFF
	        cmdTbl[7] = val & 0xFF
	        val = ctrl["green"]
	        cmdTbl[8] = (val >> 8) & 0xFF
	        cmdTbl[9] = val & 0xFF
	        val = ctrl["blue"]
	        cmdTbl[10] = (val >> 8) & 0xFF
	        cmdTbl[11] = val & 0xFF
	        val = ctrl["wifimode"]
	        cmdTbl[12] = val & 0xFF
			print("user w:")
			LOGTBL(cmdTbl)
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
	local status = '{"light":%d,"mode":%d,"timeout":%d,"brightness":%d,"red":%d,"green":%d,"blue":%d,"service":%d,"wifimode":%d}'
    local PRI2STD_LEN_NONE = 0x00000000
    local PRI2STD_LEN_INTERNAL = 0x40000000
    local PRI2STD_LEN_BOTH = 0x20000000
    local PRI2STD_LEN_MAX = 0x0000FFFF
    local v1, v2, v3, v4, v5, v6, v7, v8, v9
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

			if nil ~= string.find(bin, string.char(0x01, 0x80, 0x00, 0x00, 0x04)) and 
				dataTbl[10] == 0x49 then
				v1, v2, v3, v4, v5, v6, v7, v8 = 0, 0, 0, 0, 0, 0, 0, 0
				local lenStatus, currStatus = s1apiGetDevStatus()
				if lenStatus <= 2 then
					str = string.format(status, 0, 0, 0, 0, 0, 0, 0, 0, 0)
				elseif lenStatus > 2 then
					local tb = cjson.decode(currStatus)
					v1 = tb["light"]
					v2 = tb["mode"]
					v3 = tb["timeout"]
					v4 = tb["brightness"]
					v5 = tb["red"]
					v6 = tb["green"]
					v7 = tb["blue"]
					v8 = tb["wifimode"]
				end
                str = string.format(status, 1, 102, 45, v4, v5, v6, v7, 0, v8)
				len = PRI2STD_LEN_INTERNAL
                break
			end
			-- (RSP) ept list {"sDevQryEpt":2,"ud":"DB8F","ept":[1,2]}
			if nil ~= string.find(bin, string.char(0x01, 0x80, 0x45)) then
				str = '{"sDevQryEpt":2,"ud":"'..bin2hex(string.sub(bin,9,10))..'","ept":['
				for i = 12, dataTbl[11]+11 do
					str = str..dataTbl[i]..','
				end
				str = string.sub(str,1,string.len(str) - 1)..']}'
				break
			end
			
			-- (RSP) ept info {"sDevQryEptInfo":2,"ud":"DB8F","pid":"0104","ept":1,"did":"0101","clu":["0000","0004"]}
			if nil ~= string.find(bin, string.char(0x01, 0x80, 0x43)) then
				str = '{"sDevQryEptInfo":2,"ud":"'..bin2hex(string.sub(bin,9,10))..'","ept":'..dataTbl[12]..',"sDevDes":{"ept":'..dataTbl[12]..',"pid":"'..bin2hex(string.sub(bin,13,14))..'","did":"'..bin2hex(string.sub(bin,15,16))..'"'
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
				str = str..'}}'
				-- if nil ~= string.find(string.char(0x04, 0x02), string.sub(bin,15,16)) then
				-- 	len = PRI2STD_LEN_BOTH
				-- end
				break
			end

			-- (RSP) manufacture {"sDevQryMan":2,"ud":"DB8F","man":"1234"}
			if nil ~= string.find(bin, string.char(0x01, 0x80, 0x42)) then
				local ud = bin2hex(string.sub(bin,9,10))
				local man = bin2hex(string.sub(bin,11,12))
				
				-- test only START
				local mac = s1apiSDevGetMacByUserData(bin2hex(string.sub(bin,9,10)))
				-- local mac = "00124B000CC39852"
				if string.find(mac, "00124B00076A76E4") then
					man = bin2hex(string.char(0x00, 0x1C))
				elseif string.find(mac, "00124B000CC39852") then
					man = bin2hex(string.char(0x00, 0x1D))
				elseif string.find(mac, "00158D0000F4D4E7") then
					man = bin2hex(string.char(0xEE, 0xFF))
				elseif string.find(mac, "00124B000DC6A96C") then
					man = bin2hex(string.char(0x00, 0x1D))
				elseif string.find(mac, "00124B000DC03C5F") then
					man = bin2hex(string.char(0x00, 0x1D))
				end
				-- test only END
				str = '{"sDevQryMan":2,"ud":"'..ud..'","man":"'..man..'"}'
				break
			end

			-- INTERNAL -> EXTERNAL
			-- (IND) join, leave
			if nil ~= string.find(bin, string.char(0x01, 0x00, 0x4D)) then
				-- {"sDevJoin":2,"sDev":{"ud":"DB8F","mac":"6FE34CE400A06FC0"}}
				str = '{"sDevJoin":2,"ud":'..'"'..bin2hex(string.sub(bin,7,8))..'"'..',"mac":'..'"'..bin2hex(string.sub(bin,9,16))..'"'..'}'
				-- str = '{"sDevJoin":2,"sDev":{"ud":"DB8F","mac":'..'"'..bin2hex(string.sub(bin,9,16))..'"'..'}}'
				break
			end
			if nil ~= string.find(bin, string.char(0x01, 0x80, 0x48)) then
				-- {"sDevDel":2,"mac":"6FE34CE400A06FC0"}
				str = '{"sDevDel":2,"mac":'..'"'..bin2hex(string.sub(bin,7,14))..'"'..'}'
				break
			end
			-- (IND) ind actions
			-- {"sDevStatus":{"btn":1},"sDev":{"ud":"DB8F"}}
			if nil ~= string.find(bin, string.char(0x01, 0x81, 0x02)) then
				str = '{"sDevStatus":'..genStatus(bin2hex(string.sub(bin,11,12)), bin2hex(string.sub(bin,10,10)), bin2hex(string.sub(bin,17,17)))..',"sDev":{"ud":"'..bin2hex(string.sub(bin,8,9)).. '"}}'
				break
			end

			-- (IAS IND) 018401000da50401050002c5eb00000000000103 
			if nil ~= string.find(bin, string.char(0x01, 0x84, 0x01)) then
				str = '{"sDevStatus":'..genStatus(bin2hex(string.sub(bin,9,10)), bin2hex(string.sub(bin,8,8)), bin2hex(string.sub(bin,14,15)))..',"sDev":{"ud":"'..bin2hex(string.sub(bin,12,13)).. '"}}'
				break
			end
			-- 01 80 10 00 04 97 00 02 00 01 03 
			-- 01 80 10 02 10 02 14 97 02 10 02 12 02 10 02 01 03
			-- 0180100210021497021002120210020103
			-- ver = sdk + app
			if nil ~= string.find(bin, string.char(0x01, 0x80, 0x10)) then
				-- str = '{"sDevVer":"'..bin2hex(string.sub(bin,9,10))..bin2hex(string.sub(bin,7,8))..'"}'
				local a = bin2hex(string.sub(bin,7,7)) | ((bin2hex(string.sub(bin,8,8))) << 24) | bin2hex(string.sub(bin,9,9)) | (bin2hex(string.sub(bin,10,10)) << 8)
				str = '{"sDevVer":'..a..'}'
				break
			end
		end
		print("result -> "..str..' len '..len..'\r\n')
	elseif 0x20 == cvtType then
		v1, v2, v3, v4, v5, v6, v7, v8 = 0, 0, 0, 0, 0, 0, 0, 0
		local lenStatus, currStatus = s1apiGetDevStatus()
		if lenStatus <= 2 then
			str = string.format(status, 0, 0, 0, 0, 0, 0, 0, 0, 0)
		end
		local id = bin:byte(1)
		local i = 0, v
		if id == 0 then
            if lenStatus > 2 then
                local tb = cjson.decode(currStatus)
                v1 = tb["light"]
                v2 = tb["mode"]
                v3 = tb["timeout"]
                v4 = tb["brightness"]
                v5 = tb["red"]
                v6 = tb["green"]
                v7 = tb["blue"]
                v8 = tb["wifimode"]
            end
            if bin:byte(2) == 1 then
                if v1 == 0 then
                    str = string.format(status, 1, v2, 0, v4, v5, v6, v7, 0, v8)
                else
                    str = string.format(status, 0, v2, 0, v4, v5, v6, v7, 0, v8)
                end
            elseif bin:byte(2) == 2 then
                str = string.format(status, 1, 100, 10, v4, v5, v6, v7, 1, v8)
            elseif bin:byte(2) == 0xFF then
                str = string.format(status, 1, 101, 10, v4, v5, v6, v7, 0xFF, v8)
            elseif bin:byte(2) == 0 then
                str = string.format(status, v1, v2, v3, v4, v5, v6, v7, 0, v8)
            end
        end
		len = PRI2STD_LEN_BOTH
	elseif 0x8000 == cvtType then
		if #bin >= 11 then
			v8, v9 = 0, 0
            local lenStatus1, currStatus1 = s1apiGetDevStatus()
            if lenStatus1 > 2 then
	            local tb1 = cjson.decode(currStatus1)
	            v8 = tb1["service"]
        	end
			v1 = bin:byte(1)
	        v2 = bin:byte(2)
	        v3 = bin:byte(3)
	        v4 = bin:byte(4)
	        v4 = (v4 << 8) | bin:byte(5)
	        v5 = bin:byte(6)
	        v5 = (v5 << 8) | bin:byte(7)
	        v6 = bin:byte(8)
	        v6 = (v6 << 8) | bin:byte(9)
	        v7 = bin:byte(10)
	        v7 = (v7 << 8) | bin:byte(11)
	        v9 = bin:byte(12)
	        str = string.format(status, v1, v2, v3, v4, v5, v6, v7, v8, v9)
		end
	end
	return string.len(str) + len, str
end

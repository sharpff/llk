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

function calFCS(data)
    local result = 0x00;
	for i = 2, #data - 1 do
		result = result ~ data[i]
	end
    return result
end

--[[ EXTERNAL
	s1GetVer
  ]]
function s1GetVer()
	-- local cmdTbl = {0xFE, 0x0E, 0x29, 0x00, 0x0B, 0x0B, 0x5f, 0x01, 0x08, 0x00, 0x07, 0x02, 0x11, 0x01, 0x00, 0x01, 0x00, 0x00, 0x6D}
	local cmdTbl = {0xFE, 0x0D, 0x29, 0x00, 0x0B, 0x02, 0x00, 0x0B, 0xFF, 0xFF, 0x06, 0x02, 0x00, 0x00, 0x02, 0x3C, 0x00, 0x00}
	cmdTbl[#cmdTbl] = calFCS(cmdTbl)
	LOGTBL(cmdTbl)

	-- local tblData1 = {0x01, 0x80, 0x00, 0x00, 0x04, 0x99, 0x00, 0x54, 0x00, 0x49, 0x03}
	-- LOGTBL(tblData1)
	-- LOGTBL(whatWrite(tblData1))

	-- local tblData2 = {0x01, 0x02, 0x10, 0x49, 0x02, 0x10, 0x02, 0x14, 0x46, 0xFF, 0xFC, 0x02, 0x18, 0x02, 0x10, 0x03}
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
	local tblDataCountLen = {}
	local strDataCountLen = ""
	local where = 1
	local idx = 0
	local tblData = stringToTable(data)
	print("[LUA] total ======> "..#tblData.."\r\n")
	LOGTBL(tblData)
	print("[LUA] <============ \r\n")

	while where < #tblData do
		local tmpLen = tblData[where + 1]
		print("tmpLen is "..tmpLen.."\r\n")
		local tmpString = string.sub(data, where, (where + 4 + tmpLen))
		local tmpTbl = stringToTable(tmpString)
		LOGTBL(tmpTbl)
		if tblData[where + 4 + tmpLen] ~= calFCS(tmpTbl) then
			print("[LUA E] calFCS failed\r\n")
			break
		end
		tblDataCountLen[idx + 1] = #tmpTbl & 0xFF
		tblDataCountLen[idx + 2] = 0x00
		idx = idx + 2
		where = where + 4 + tmpLen + 1
		print("[LUA] IDX @"..where.." & LEN is "..tmpLen.." ------->\r\n")
		LOGTBL(tmpTbl)
		print("[LUA] --------------------------------- \r\n")
	end

	print("[LUA] out ========> \r\n")
	LOGTBL(tblDataCountLen)
	print("[LUA] <============ \r\n")
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

	local dataTbl = stringToTable(data)
	LOGTBL(dataTbl)

	-- test only
	cvtType = 0x01

	for i = 1, 1 do
		-- UART
		if 0x01 == cvtType then
			-- START for status of sub devices
			if dataTbl[3] == 0x45 and dataTbl[4] == 0xC1 then
				-- (IND) new device joining, RAW ind is 0102104D02100B2331BC60AF76337EE10219748003
				print ("[LUA] s1GetValidKind - new device joining\r\n")
				ret = WHATKIND_SUB_DEV_JOIN
				break
			end

			if dataTbl[3] == 0x45 and dataTbl[4] == 0xC9 then
				-- (IND) new device leaving, RAW ind is 01804802100219A960AF76337EE1021974021003 
				print ("[LUA] s1GetValidKind - new device leaving\r\n")
				ret = WHATKIND_SUB_DEV_LEAVE
				break
			end

			if dataTbl[3] == 0x45 and dataTbl[4] == 0x85 or
				dataTbl[3] == 0x45 and dataTbl[4] == 0x84 or
				dataTbl[3] == 0x45 and dataTbl[4] == 0x82 then
				-- (RSP) query ept, query ept info, query man
				print ("[LUA] s1GetValidKind - ENDPOINT list or info "..#dataTbl.."\r\n")
				ret = WHATKIND_SUB_DEV_INFO
				break
			end

			if nil ~= string.find(dataTbl, string.char(0x01, 0x81, 0x02)) then
				-- (IND) sDevStatus, RAW ind 0181021202100B2D0212853002150210021602100210021010021103 
				print ("[LUA] s1GetValidKind - sDevStatus ind "..#dataTbl.."\r\n")
				ret = WHATKIND_SUB_DEV_DATA
				break
			end

			-- TODO: if it is really need
			if nil ~= string.find(dataTbl, string.char(0x01, 0x80, 0x06, 0x00, 0x01, 0x86, 0x01, 0x03)) or
				nil ~= string.find(dataTbl, string.char(0X01, 0X80, 0X00, 0X00, 0X04)) then
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
	local sDevCtrl = ctrl["sDevCtrl"]
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

	end

	cmdTbl = whatWrite(cmdTbl)
	-- LOGTBL(cmdTbl)
	dataStr = tableToString(cmdTbl)
	return string.len(dataStr), dataStr
end

--[[ EXTERNAL
	s1CvtPri2Std
  ]]
function s1CvtPri2Std(bin)
	local cvtType = s1apiGetCurrCvtType()
	local dataTbl = {}
	-- local strSubDev = '"sDev":{"pid":"%s","clu":"%s","ept":%s,"mac":"%s"}'
	local strSubDev = '{}'
	dataTbl = stringToTable(bin)

	dataTbl = whatRead(dataTbl)
	print("s1CvtPri2Std\r\n")
	LOGTBL(dataTbl)
	bin = tableToString(dataTbl)

	-- test only
	cvtType = 0x01

	for i = 1, 1 do
		-- UART
		if 0x01 == cvtType then
			-- INTERNAL
			-- (RSP) ept list {"sDevQryEpt":2,"idx":"DB8F","ept":[1,2]}
			if nil ~= string.find(bin, string.char(0x01, 0x80, 0x45)) then
				strSubDev = '{"sDevQryEpt":2,"idx":"'..bin2hex(string.sub(bin,9,10))..'","ept":['
				for i = 12, dataTbl[11]+11 do
					strSubDev = strSubDev..dataTbl[i]..','
				end
				strSubDev = string.sub(strSubDev,1,string.len(strSubDev) - 1)..']}'
				break
			end
			
			-- (RSP) ept info {"sDevQryEptInfo":2,"idx":"DB8F","pid":"0104","ept":1,"did":"0101","clu":["0000","0004"]}
			if nil ~= string.find(bin, string.char(0x01, 0x80, 0x43)) then
				strSubDev = '{"sDevQryEptInfo":2,"idx":"'..bin2hex(string.sub(bin,9,10))..'","pid":"'..bin2hex(string.sub(bin,13,14))..'","ept":'..dataTbl[12]..',"did":"'..bin2hex(string.sub(bin,15,16))..'","clu":['
				for i = 19, dataTbl[18]*2+18, 2 do
					strSubDev = strSubDev..'"'..bin2hex(string.sub(bin,i,i+1))..'",'
				end
				strSubDev = string.sub(strSubDev,1,string.len(strSubDev) - 1)..']}'
				break
			end

			-- (RSP) manufacture {"sDevQryMan":2,"idx":"DB8F","man":"1234"}
			if nil ~= string.find(bin, string.char(0x01, 0x80, 0x42)) then
				strSubDev = '{"sDevQryMan":2,"idx":"'..bin2hex(string.sub(bin,9,10))..'","man":"'..bin2hex(string.sub(bin,11,12))..'"}'
				break
			end

			-- INTERNAL -> EXTERNAL
			-- (IND) join, leave
			if nil ~= string.find(bin, string.char(0x01, 0x00, 0x4D)) then
				-- {"sDevJoin":2,"sDev":{"idx":"DB8F","mac":"6FE34CE400A06FC0"}}
				strSubDev = '{"sDevJoin":2,"sDev":{"idx":'..'"'..bin2hex(string.sub(bin,7,8))..'"'..',"mac":'..'"'..bin2hex(string.sub(bin,9,16))..'"'..'}}'
				-- strSubDev = '{"sDevJoin":2,"sDev":{"idx":"DB8F","mac":'..'"'..bin2hex(string.sub(bin,9,16))..'"'..'}}'
				break
			end
			if nil ~= string.find(bin, string.char(0x01, 0x80, 0x48)) then
				-- {"sDevLeave":2,"sDev":{mac":"6FE34CE400A06FC0"}}
				strSubDev = '{"sDevLeave":2,"sDev":{"mac":'..'"'..bin2hex(string.sub(bin,7,14))..'"'..'}}'
				break
			end
			-- (IND) ind actions
			-- {"sDevStatus":{"btn":1},"sDev":{"idx":"DB8F"}}
			if nil ~= string.find(bin, string.char(0x01, 0x81, 0x02)) then
				strSubDev = '{"sDevStatus":'..genStatus(bin2hex(string.sub(bin,11,12)), bin2hex(string.sub(bin,10,10)), bin2hex(string.sub(bin,17,17)))..',"sDev":{"idx":"'..bin2hex(string.sub(bin,8,9)).. '"}}'
				break
			end

		end

	end
	print("result -> "..strSubDev..'\r\n')
	return string.len(strSubDev), strSubDev
end

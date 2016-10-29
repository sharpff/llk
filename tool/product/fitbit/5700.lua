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
	-- local cmdTbl = {0xFE, 0x0B, 0x29, 0x00, 0x0B, 0xEC, 0x12, 0x0B, 0x06, 0x00, 0x04, 0x02, 0x11, 0x01, 0x01, 0x00}
	-- local cmdTbl = {0xFE, 0x0F, 0x29, 0x00, 0x0B, 0xEC, 0x12, 0x0B, 0x00, 0x03, 0x08, 0x02, 0x11, 0x00, 0x0a, 0xA6, 0x00, 0x00, 0x00, 0x00}
	-- cmdTbl[#cmdTbl] = calFCS(cmdTbl)
	-- LOGTBL(cmdTbl)

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
			"baud":"57600-8N1"
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

	local dataTbl = stringToTable(data)
	LOGTBL(dataTbl)

	-- test only
	cvtType = 0x01

	for i = 1, 1 do
		-- UART
		if 0x01 == cvtType then
			-- START for status of sub devices
			if dataTbl[3] == 0x45 and dataTbl[4] == 0xC1 then
				print ("[LUA] s1GetValidKind - new device joining\r\n")
				ret = WHATKIND_SUB_DEV_JOIN
				break
			end

			if dataTbl[3] == 0x45 and dataTbl[4] == 0xC9 then
				print ("[LUA] s1GetValidKind - new device leaving\r\n")
				ret = WHATKIND_SUB_DEV_LEAVE
				break
			end

			-- (RSP) query ept, query ept info, query man
			if dataTbl[3] == 0x45 and dataTbl[4] == 0x85 or
				dataTbl[3] == 0x45 and dataTbl[4] == 0x84 or
				dataTbl[3] == 0x45 and dataTbl[4] == 0x82 then
				print ("[LUA] s1GetValidKind - ENDPOINT list or info "..#dataTbl.."\r\n")
				ret = WHATKIND_SUB_DEV_INFO
				break
			end

			if dataTbl[3] == 0x49 and dataTbl[4] == 0x83 then
				print ("[LUA] s1GetValidKind - sDevStatus ind "..#dataTbl.."\r\n")
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
    return string.reverse(s)
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
			if ctrl["reset"] == 1 then
				cmdTbl = {0xFE, 0x0D, 0x29, 0x00, 0x0B, 0x02, 0x00, 0x0B, 0xFF, 0xFF, 0x06, 0x02, 0x00, 0x00, 0x02, 0x3C, 0x00, 0x00}
				break
			end

			if ctrl["sDevJoin"] == 1 then
				cmdTbl = {0xFE, 0x0D, 0x29, 0x00, 0x0B, 0x02, 0x00, 0x0B, 0xFF, 0xFF, 0x06, 0x02, 0x00, 0x00, 0x05, 0x3C, 0x01, 0x1A}
 				break
			end

			-- "{\"sDevLeave\":1,\"idx\":\"ABCD\",\"mac\":\"C06FA000E44CE36F\"}"
			if ctrl["sDevLeave"] then
				local addrHex = hex2bin(ctrl["idx"])
				local macHex = hex2bin(ctrl["mac"])
				cmdTbl = {0xFE, 0x13, 0x29, 0x00, 0x0B, 0x00, 0x00, 0x0C, 0xFF, 0xFF, 0x0C, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36}
				cmdtbl[6] = addrHex[1]
				cmdtbl[7] = addrHex[2]
				cmdtbl[16] = macHex[1]
				cmdtbl[17] = macHex[2]
				cmdtbl[18] = macHex[3]
				cmdtbl[19] = macHex[4]
				cmdtbl[20] = macHex[5]
				cmdtbl[21] = macHex[6]
				cmdtbl[22] = macHex[7]
				cmdtbl[23] = macHex[8]
				LOGTBL(cmdTbl)
 				break
			end

			-- TODO: ctrl the sub dev.
			if sDevCtrl then
				cmdTbl = {0xfe, 0x00, 0x29, 0x00, 0x0b, 0x00, 0x00, 0x01, 0x00, 0x00, 0x07, 0x02, 0x11, 0x01}
				local mac = ctrl["sDev"]["mac"]
				local dEpt = ctrl["sDev"]["ept"]
				-- local idx = s1apiSdevGetUserDataByMac(mac)
				local idx = string.char(0xec, 0x12)
				local addrHex = hex2bin(idx)
				cmdtbl[6] = addrHex[1]
				cmdtbl[7] = addrHex[2]
				if sDevCtrl["onOff"] then
					cmdtbl[9] = 0x06
					cmdtbl[10] = 0x00
					cmdtbl[15] = sDevCtrl["onOff"]
				elseif sDevCtrl["moveToLevel"] then
					cmdtbl[9] = 0x08
					cmdtbl[10] = 0x00
					cmdtbl[15] = 0x04
					cmdtbl[16] = 0xFF -- val
					cmdtbl[17] = 0x00 -- timeL
					cmdtbl[18] = 0x00 -- timeH
				elseif sDevCtrl["H&S"] then
					cmdtbl[9] = 0x00
					cmdtbl[10] = 0x03
					cmdtbl[15] = 0x06
					cmdtbl[16] = 0xFF -- val
					cmdtbl[17] = 0x00 -- H
					cmdtbl[18] = 0x00 -- S
					cmdtbl[19] = 0x00 -- timeL
					cmdtbl[20] = 0x00 -- timeH
				elseif sDevCtrl["temp"] then
					cmdtbl[9] = 0x00
					cmdtbl[10] = 0x03
					cmdtbl[15] = 0x0a
				end
				cmdTbl[#cmdTbl] = 0x00
				cmdTbl[#cmdTbl-1] = calFCS(cmdTbl)
				LOGTBL(cmdTbl)
				break
			end

			if itself then
				print("TODO: ctrl the self dev. \r\n")
				break
			end
		end

	end

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

	-- test only
	cvtType = 0x01

	for i = 1, 1 do
		-- UART
		if 0x01 == cvtType then
			-- INTERNAL
			-- (RSP) ept list {"sDevQryEpt":2,"idx":"DB8F","ept":[1,2]}
			if dataTbl[3] == 0x45 and dataTbl[4] == 0x85 then
				strSubDev = '{"sDevQryEpt":2,"idx":"'..bin2hex(string.sub(bin,5,6))..'","ept":['
				for i = 11, dataTbl[10]+10 do
					strSubDev = strSubDev..dataTbl[i]..','
				end
				strSubDev = string.sub(strSubDev,1,string.len(strSubDev) - 1)..']}'
				break
			end
			
			-- (RSP) ept info {"sDevQryEptInfo":2,"idx":"DB8F","pid":"0104","ept":1,"did":"0101","clu":["0000","0004"]}
			if dataTbl[3] == 0x45 and dataTbl[4] == 0x84 then
				strSubDev = '{"sDevQryEptInfo":2,"idx":"'..bin2hex(string.sub(bin,5,6))..'","pid":"'..bin2hex(string.sub(bin,12,13))..'","ept":'..dataTbl[11]..',"did":"'..bin2hex(string.sub(bin,14,15))..'","clu":['
				local sClu = 18
				if dataTbl[sClu-1] > 0 then
					for i = sClu, dataTbl[sClu-1]*2+sClu-1, 2 do
						strSubDev = strSubDev..'"'..bin2hex(string.sub(bin,i,i+1))..'",'
					end
				end
				sClu = sClu + dataTbl[sClu-1]*2 + 1
				if dataTbl[sClu-1] > 0 then
					for i = sClu, dataTbl[sClu-1]*2+sClu-1, 2 do
						strSubDev = strSubDev..'"'..bin2hex(string.sub(bin,i,i+1))..'",'
					end
				end
				strSubDev = string.sub(strSubDev,1,string.len(strSubDev)-1)..']}'
				break
			end

			-- (RSP) manufacture {"sDevQryMan":2,"idx":"DB8F","man":"1234"}
			if dataTbl[3] == 0x45 and dataTbl[4] == 0x82 then
				strSubDev = '{"sDevQryMan":2,"idx":"'..bin2hex(string.sub(bin,5,6))..'","man":"'..bin2hex(string.sub(bin,13,14))..'"}'
				break
			end

			-- INTERNAL -> EXTERNAL
			-- (IND) join, leave
			if dataTbl[3] == 0x45 and dataTbl[4] == 0xC1 then
				-- {"sDevJoin":2,"sDev":{"idx":"DB8F","mac":"6FE34CE400A06FC0"}}
				strSubDev = '{"sDevJoin":2,"sDev":{"idx":'..'"'..bin2hex(string.sub(bin,5,6))..'"'..',"mac":'..'"'..bin2hex(string.sub(bin,9,16))..'"'..'}}'
				break
			end
			if dataTbl[3] == 0x45 and dataTbl[4] == 0xC9 then
				-- {"sDevLeave":2,"sDev":{mac":"6FE34CE400A06FC0"}}
				strSubDev = '{"sDevLeave":2,"sDev":{"mac":'..'"'..bin2hex(string.sub(bin,5,6))..'"'..'}}'
				break
			end
			-- (IND) ind actions
			-- {"sDevStatus":{"btn":1},"sDev":{"idx":"DB8F"}}
			if dataTbl[3] == 0x49 and dataTbl[4] == 0x83 then
				strSubDev = '{"sDevStatus":'..genStatus(bin2hex(string.sub(bin,8,9)), bin2hex(string.sub(bin,7,7)), bin2hex(string.sub(bin,14,15)))..',"sDev":{"idx":"'..bin2hex(string.sub(bin,5,6)).. '"}}'
				break
			end

		end

	end
	print("result -> "..strSubDev..'\r\n')
	return string.len(strSubDev), strSubDev
end

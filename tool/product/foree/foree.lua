--[[ 
	THIS IS FW SCRIPT abc
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

function LOGTBL(tblObj)
	local str = ""
	for i = 1, #tblObj do 
		str = str..string.format('%02x ', tblObj[i])
	end
	print ('LOGTBL '..str..'\r\n')
end

--[[ MUST
	0. UART json <-> bin
	1. PIPE/IPC json <-> json
]]
function s1GetVer()
	-- body
	local str = '1.0'
	return string.len(str), str
end

--[[ MUST
	whatCvtType:
	0. UART json <-> bin
	1. PIPE/IPC json <-> json
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
    	],
    "gpio":[
	        {
	            "id":1,
	            "dir":0,
	            "mode":2,
	            "type":1,
	            "longTime":10,
	            "shortTime":3
	        },
	        {
	            "id":2,
	            "dir":1,
	            "mode":0,
	            "state":1,
	            "blink":2,
	            "type":1,
	            "longTime":10,
	            "shortTime":1
	        }
	    ]
	}
    ]]
	local delay = 5

	return string.len(str), str, delay
end

-- function s1HasSubDevs() 
-- 	return 1
-- end

--[[ MUST
	
]]
function s1GetQueries(queryType)
	local cvtType = s1apiGetCurrCvtType()
	local query = ''
	local queryCountLen = ''

	-- test only
	cvtType = 1

	print ("s1GetQueries cvtType is " .. cvtType .. ", queryType is " .. queryType .."\r\n")

	for i = 1, 1 do
		-- UART
		if 0x01 == cvtType then

			-- for status indication
			if 0 ~= (queryType & 0x000000FF) then
				print ("status indication \r\n")
				break
			end

			-- START for status of itself
			if 1 == ((queryType & 0x0000FF00) >> 8) then
				-- reset itself, is as the same as BEFORE fw script
				print ("for status of itself - reset itself\r\n")
				break
			end
			-- ********************** this will be trigger by ctrl cmd ***********************
			-- if 2 == ((queryType & 0x0000FF00) >> 8) then
			-- 	-- reset gw
			-- 	query = string.char(0xAA, 0x00, 0x04, 0x02, 0x00, 0x00, 0x00, 0xDB)
			-- 	queryCountLen = string.char(0x08, 0x00)
			-- 	print ("for status of itself - reset gw\r\n")
			-- 	break
			-- end
			-- if 3 == ((queryType & 0x0000FF00) >> 8) then
			-- 	-- classical join
			-- 	query = string.char(0xAA, 0x00, 0x0D, 0x02, 0x04, 0x01, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7B)
			-- 	queryCountLen = string.char(0x11, 0x00)
			-- 	print ("for status of itself - classical join\r\n")
			-- 	break
			-- end
			-- END for status of itself


			-- if a == 1 then
			-- 	do()
			-- elseif a == 2 then
			-- 	do()
			-- elseif a == 3 then
			-- 	do()
			-- else
			-- 	do()
			-- end
		end

		-- GPIO
		if 0x02 == cvtType then

		end
	end

	return string.len( queryCountLen ), queryCountLen, string.len( query ), query
end

--[[ MUST

]]

function s1GetValidKind(data)
	local cvtType = s1apiGetCurrCvtType()
	local ret = 0
	local dataTbl = nil
	local WHATKIND_MAIN_DEV_RESET = 1
	local WHATKIND_MAIN_DEV_DATA = 2
	local WHATKIND_SUB_DEV = 10

	-- test only
	cvtType = 1

	for i = 1, 1 do
		-- UART
		if 0x01 == cvtType then
			dataTbl = stringToTable(data)
			-- START for status of sub devices
			if nil ~= string.find(data, string.char(0xAA, 0x00, 0x05, 0x82, 0x00, 0x00, 0x00, 0x00, 0xB1, 0xFF, 0xFF)) then
				-- (RSP) reset rsp , rep is AA 00 04 02 00 00 00 DB
				print ("s1GetValidKind - sub devices - reset rsp\r\n")
				ret = WHATKIND_SUB_DEV
				break
			end

			if nil ~= string.find(data, string.char(0xAA, 0x00, 0x05, 0x82, 0x04, 0x01, 0x41, 0x00, 0xCC)) then
				-- (RSP) join permition rsp , rep is AA 00 0D 02 04 01 41 00 00 00 00 00 00 00 00 00 7B
				print ("s1GetValidKind - sub devices - join permition rsp\r\n")
				ret = WHATKIND_SUB_DEV
				break
			end

			if nil ~= string.find(data, string.char(0xAA, 0x00, 0x13, 0x81, 0x10)) then
				-- (RSP) list rsp , rep is AA 00 02 01 10 9D
				print ("s1GetValidKind - sub devices - list rsp\r\n")
				ret = WHATKIND_SUB_DEV
				break
			end

			if dataTbl[1] == 0xAA and dataTbl[2] == 0x00 and dataTbl[4] == 0x81 and dataTbl[5] == 0x00 then
				-- (RSP) device info rsp , rep is AA 00 04 01 00 00 00 E1
				print ("s1GetValidKind - sub devices - device info rsp "..#dataTbl.."\r\n")
				ret = WHATKIND_SUB_DEV
				break
			end

			if nil ~= string.find(data, string.char(0xAA, 0x00, 0x11, 0x82)) then
				-- (IND) new device joining, rep is 0xAA, 0x00, 0x11, 0x82
				print ("s1GetValidKind - sub devices - new device joining\r\n")
				ret = WHATKIND_SUB_DEV
				break
			end

			if dataTbl[1] == 0xAA and dataTbl[2] == 0x00 and dataTbl[4] == 0x90 then
				-- (IND) sensor action ind
				print ("s1GetValidKind - sub devices - sensor action ind "..#dataTbl.."\r\n")
				ret = WHATKIND_SUB_DEV
				break
			end

		end

		-- GPIO
		if 0x02 == cvtType then
			return WHATKIND_MAIN_DEV_DATA
		end
	end
	-- print '0'
	-- invalid kind
	return ret
end


-- {"msg":"hello","sDev":{"pid":"0104","did":"0107","clu":"0006","ept":[1, 2],"mac":"7409E17E3376AF60"}}
-- {"ctrl":{"pwr":1},"sDev":{"pid":"0104","did":"0107","clu":"0006","ept":[1, 2],"mac":"7409E17E3376AF60"}}
-- {"status":{"pwr":1,"switcher":1},"sDev":{"pid":"0104","did":"0107","clu":"0006","ept":[1, 2],"mac":"7409E17E3376AF60"}}
-- s1apiGetMacFromIdx()
-- s1apiGetIdxFromMac()

-- \{\"ctrl\":\{\"reset\":1\}\}
-- \{\"ctrl\":\{\"cjoin\":1\}\}
-- \{\"ctrl\":\{\"subDevGetList\":1\}\}
-- \{\"ctrl\":\{\"subDevGetList\":1\}\}

--[[ MUST
]]
function s1CvtStd2Pri(json)
	local cvtType = s1apiGetCurrCvtType()
	-- print ('s1CvtStd2Pri return => '..cvtType..'\r\n')
	local tb = cjson.decode(json)
	local ctrl = tb["ctrl"]
	local cmdTbl = nil
	local dataStr = ""

	-- test only
	cvtType = 1

	for i = 1, 1 do
		-- UART
		if 0x01 == cvtType then
			if (ctrl["reset"] == 1) then
				cmdTbl = {0xAA, 0x00, 0x04, 0x02, 0x00, 0x00, 0x00, 0xDB}
				break
			end

			if (ctrl["cjoin"] == 1) then
				cmdTbl = {0xAA, 0x00, 0x0D, 0x02, 0x04, 0x01, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7B}
 				break
			end

			if (ctrl["subDevGetList"] == 1) then
				cmdTbl = {0xAA, 0x00, 0x02, 0x01, 0x10, 0x9D}
				break
			end
		end

		-- GPIO
		if 0x02 == cvtType then
		end
	end

	-- -- 打开
	-- if (ctrl["action"] == 1) then
	-- 	cmdTbl[2] = 0x02
	-- -- 关闭
	-- elseif (ctrl["action"] == 2) then
	-- 	cmdTbl[2] = 0x01
	-- -- 暂停
	-- elseif (ctrl["action"] == 3) then
	-- 	cmdTbl[2] = 0x03
	-- -- 测量
	-- else
	-- 	cmdTbl[2] = 0x05
	-- end
	-- LOGTBL(cmdTbl)

	-- u have to make the bin as string for the return value
	dataStr = tableToString(cmdTbl)
	return string.len(dataStr), dataStr
end

--[[ MUST
	return value: return 0, 0, if the input param 'bin' is not valid
]]
function s1CvtPri2Std(bin)
	local cvtType = s1apiGetCurrCvtType()
	-- print ('s1CvtPri2Std return => '..cvtType..'\r\n')
	local dataTbl = {}
	local str = '{"percentage":%d}'
	dataTbl = stringToTable(bin)
	-- for i = 1, #bin
	-- 		 (bin[i])
	-- end
	-- LOGTBL(dataTbl)

	str = string.format(str, 100 - dataTbl[3])
	-- str = string.format(str, #dataTbl)
	-- print (str)
	return string.len(str), str
end

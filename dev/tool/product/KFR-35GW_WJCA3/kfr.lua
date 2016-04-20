--[[ 
	THIS IS FW SCRIPT
	1000010011100081000800504321FBE7
  ]]

--[[ 
	Global values	
--]]
random_num = 1

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
	print ('---> YIP LOGTBL '..str..'\r\n')
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

	local whatCvtType = 0
	-- delay time (ms) for the interval during write & read. 
	local delay = 5
	--[[
		E.g."9600-8N1"
		FORMAT [baud(9600, ...) - dataBits(8, 9, ...) parity(None:0, Odd:1, Even:2) stopBits(1, 2)]
		refer to func(halIO.c)
		void *halUartOpen(int baud, int dataBits, int stopBits, int parity, int flowCtrl);
	]] 
	local baud = '"9600-8N1"'
	local str = string.format('{"whatCvtType":%d,"baud":%s}', whatCvtType, baud)

	return string.len(str), str, delay
end

--[[  MUST
	Generate crc	
--]]
function msGenCrc8Value(tbl, len)
	local i = 1
	local loop = 1
	local crc = 0

	for loop = 1, len do
		crc = crc ~ tbl[loop + 10]
		for i = 1, 8 do
			if crc & 0x01 == 0x01 then
				crc = (crc >> 1) ~ 0x8c
			else
				crc = crc >> 1
			end
		end
	end

	return crc
end

--[[  MUST
	Generate chksum
--]]
function msGenChksumValue(tbl)
	local i = 1
	local sum = 0
	
	for i = 1, 34 do
		sum = sum + tbl[i + 1]
	end

	sum = ~sum + 1

	return sum & 0xff
end

--[[ MUST
	鏌ヨ璁惧鐘舵€併€?
	姣忎釜璁惧閮界害瀹氶渶瑕佷竴鏉℃垨鑰呭鏉℃寚浠ゅ彲浠ヨ幏鍙栧埌璁惧鐨勬墍鏈夌姸鎬併€?
]]
function s1GetQueries()
	--local query = string.char( 0xbb, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfa, 0x44 )

	local cmdTbl = { 0xaa, 0x23, 0xac, 0x8f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 
	                 0x41, 0x81, 0x00, 0xff, 0x03, 0xff, 0x00, 0x02, 0x00, 0x00, 
	                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } -- the length of control pkg is 36

	random_num = random_num + 1
	if random_num == 255 then
		random_num = 1
	end

	cmdTbl[34] = random_num
	cmdTbl[35] = msGenCrc8Value(cmdTbl, 24)
	cmdTbl[36] = msGenChksumValue(cmdTbl)

	LOGTBL(cmdTbl)

	local query = tableToString(cmdTbl)

	local queryCountLen = string.char( 0x24, 0x00 )

	return string.len( queryCountLen ), queryCountLen, string.len( query ), query
end

--[[ MUST
	check the UART RX data for the following types:
	1.reset WIFI
	2.status data
]]
function s1GetValidKind(data)
-- function test_recv_pkg()
	-- local data = string.char( 0xaa, 0x23, 0xac, 0x00, 0x98, 0xc1, 0xe8, 0x03, 0x00, 0x03, 0x00, 0x00 )

	-- print (data, #data)

	--[[ MUST
		wifi reset cmd
	]]
	print ('--------> recv type start\r\n')
	tmpTbl = stringToTable(data)
	LOGTBL(tmpTbl)
	--tmpTbl = stringToTable(reset)
	--LOGTBL(tmpTbl)
	--print (string.find(data, reset))
	--print ('\r\n')

	local length = data:byte(2)
	local cmdtype = data:byte(10)

	print(length, cmdtype)	

	--if nil ~= string.find(data, reset) then
	if 0x64 == cmdtype and length >= 10 then	
		print '------> recv type 1'
		return 1
	end

	--[[ MUST
		valid LOW LEVEL status cmd
	]]
	--if 9 == #data then
	if 0x02 == cmdtype or 0x03 == cmdtype or 0x05 == cmdtype then	
		 print '------> recv type 2'
		return 2
	end
	 print '------> recv type 0'

	-- invalid kind
	return 0
end


function test_ms()
	local cmdTbl = { 0xaa, 0x23, 0xac, 0x8f, 0x00, 0x00, 0x14, 0x00, 0x00, 0x02, 
                         0x40, 0x43, 0x2a, 0x65, 0x7f, 0x7f, 0x00, 0x00, 0x00, 0x08, 
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
			 0x00, 0x00, 0x00, 0x05, 0x64, 0x8b } -- the length of control pkg is 36

	
	local crc = msGenCrc8Value(cmdTbl, 24)
	local chksum = msGenChksumValue(cmdTbl)

	local str = string.format("crc : 0x%02x chksum : 0x%02x\r\n", crc, chksum)
	print(str)
end

-- 鏉滀簹绐楀笜鐢垫満
-- bb 00 00 00 00 00 00 fa 44 涓嶆槑
-- bb 01 00 00 00 00 00 fa 44 閫嗘椂閽?鏈夌洰鐨?
-- bb 02 00 00 00 00 00 fa 44 椤烘椂閽?鏈夌洰鐨?
-- bb 05 00 00 00 00 00 fa 44 娴嬮噺
-- bb 03 00 00 00 00 00 fa 44 鏆傚仠
-- bb 06 00 00 00 00 00 fa 44 鐘舵€佽幏鍙?

--[[ MUST
]]
-- {"ctrl":{"action":1}}
-- {"ctrl":{"pwr":1, "temp":26, "mode":4, "speed":3}}
function s1CvtStd2Pri(json)
	local tb = cjson.decode(json)
	local ctrl = tb["ctrl"]

	local cmdTbl = { 0xaa, 0x23, 0xac, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 
                         0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } -- the length of control pkg is 36
	local dataStr = ""

	-- open:1/close:0
	cmdTbl[12] = ctrl["pwr"]
	cmdTbl[13] = ctrl["mode"] * 32 + ctrl["temp"] - 16
	
	--[[
		0 : invalid
		1 : auto
		2 : cold
		3 : kill humility
		4 : heat
		5 : wind
	--]]
	if ctrl["mode"] ~= 1 and ctrl["mode"] ~= 3 then
		-- 1 : low , 2 : middle, 3 : high , 4 : auto
		if ctrl["speed"] == 4 then
			cmdTbl[14] = 0x66
		elseif (ctrl["speed"] == 1) then
			cmdTbl[14] = 30
		elseif ctrl["speed"] == 2 then
			cmdTbl[14] = 50 
		elseif ctrl["speed"] == 3 then
			cmdTbl[14] = 80
		end
	else
		cmdTbl[14] = 0x65
	end

	random_num = random_num + 1
	if random_num == 255 then
		random_num = 1
	end

	cmdTbl[34] = random_num
	cmdTbl[35] = msGenCrc8Value(cmdTbl, 24)
	cmdTbl[36] = msGenChksumValue(cmdTbl)

	LOGTBL(cmdTbl)

	-- u have to make the bin as string for the return value
	dataStr = tableToString(cmdTbl)
	return string.len(dataStr), dataStr
end

--[[ MUST
	return value: return 0, 0, if the input param 'bin' is not valid
	{"pwr":1, "temp":26, "mode":4, "speed": 3, "envTemp":18}}
]]
function s1CvtPri2Std(bin)
	local dataTbl = {}
	dataTbl = stringToTable(bin)
--[[
        local dataTbl = { 0xaa, 0x20, 0xac, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02,
                          0xc0,0x01,0x2a,0x65,0x7f,0x7f,0x00,0x00,0x00,0x08,
                          0x00,0x5c,0xff,0x0e,0x00,0x00,0x00,0x00,0x00,0x00,
                          0x05,0x0f,0x5d}
--]]
	LOGTBL(dataTbl)

	local str = ''
	local temp = 26 -- target temperature
	local envTemp = 20 -- env temperature

	if dataTbl[10] == 0x05 then
		temp = ((dataTbl[12] >> 1) & 0x1f) + 12
		str = '{"pwr":%d,"temp":%d,"mode":%d,"speed":%d}'
	else
		temp = (dataTbl[13] & 0x0f) + 16
		envTemp = (dataTbl[22] - 50) >> 1
		str = '{"pwr":%d,"temp":%d,"mode":%d,"speed":%d,"envTemp":%d}'
	end
	-- 1 : low , 2 : middle, 3 : high , 4 : auto
	local pwr = dataTbl[12] & 0x01
	local mode = dataTbl[13] >> 5
	local speed = dataTbl[14] & 0x7f
	if speed == 102 or speed == 101 then
		speed = 4
	elseif speed <= 40 then
		speed = 1
	elseif speed <= 60 then
		speed = 2
	elseif speed < 101 then
		speed = 3
	end 
	
	str = string.format(str, pwr, temp, mode, speed, envTemp)
	-- str = string.format(str, 100 - dataTbl[3])
	-- str = string.format(str, #dataTbl)
	print (str)
	return string.len(str), str
end
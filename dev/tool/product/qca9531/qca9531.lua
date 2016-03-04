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

--[[ MUST
	查询设备状态。
	每个设备都约定需要一条或者多条指令可以获取到设备的所有状态。
]]
function s1GetQueries()
	local query = string.char( 0xbb, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfa, 0x44 )
	local queryCountLen = string.char( 0x09, 0x00 )

	return string.len( queryCountLen ), queryCountLen, string.len( query ), query
end

--[[ MUST
	WIFI 重置命令判别
]]
function s1GetValidKind(data)
	local reset = string.char( 0x12, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x22 )
	-- print (data, #data)

	--[[ MUST
		wifi reset cmd
	]]
	if reset == data then
		-- print '1'
		return 1
	end

	--[[ MUST
		valid LOW LEVEL status cmd
	]]
	if 9 == #data then
		-- print '2'
		return 2
	end

	-- print '0'
	-- invalid kind
	return 0
end

-- 杜亚窗帘电机
-- bb 00 00 00 00 00 00 fa 44 不明
-- bb 01 00 00 00 00 00 fa 44 逆时针 有目的
-- bb 02 00 00 00 00 00 fa 44 顺时针 有目的
-- bb 05 00 00 00 00 00 fa 44 测量
-- bb 03 00 00 00 00 00 fa 44 暂停
-- bb 06 00 00 00 00 00 fa 44 状态获取

--[[ MUST
]]
-- {"ctrl":{"action":1}}
function s1CvtStd2Pri(json)
	local tb = cjson.decode(json)
	local ctrl = tb["ctrl"]
	local cmdTbl = { 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfa, 0x44 }
	local dataStr = ""

	-- 打开
	if (ctrl["action"] == 1) then
		cmdTbl[2] = 0x02
	-- 关闭
	elseif (ctrl["action"] == 2) then
		cmdTbl[2] = 0x01
	-- 暂停
	elseif (ctrl["action"] == 3) then
		cmdTbl[2] = 0x03
	-- 测量
	else
		cmdTbl[2] = 0x05
	end
	LOGTBL(cmdTbl)

	-- u have to make the bin as string for the return value
	dataStr = tableToString(cmdTbl)
	return string.len(dataStr), dataStr
end

--[[ MUST
	return value: return 0, 0, if the input param 'bin' is not valid
]]
function s1CvtPri2Std(bin)
	local dataTbl = {}
	local str = '{"qca9531":%d,"pwr":1}'
	dataTbl = stringToTable(bin)
	-- for i = 1, #bin
	-- 		 (bin[i])
	-- end
	LOGTBL(dataTbl)

	str = string.format(str, 100 - dataTbl[3])
	-- str = string.format(str, #dataTbl)
	-- print (str)
	return string.len(str), str
end

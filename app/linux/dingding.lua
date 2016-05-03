--[[ 
	THIS IS FW SCRIPT
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
	1. GPIO
]]
function s1GetCvtType()

	local whatCvtType = 1
	-- delay time (ms) for the interval during write & read. 
	local delay = 5
	--[[
		E.g."9600-8N1"
		FORMAT [baud(9600, ...) - dataBits(8, 9, ...) parity(None:0, Odd:1, Even:2) stopBits(1, 2)]
		refer to func(halIO.c)
		void *halUartOpen(int baud, int dataBits, int stopBits, int parity, int flowCtrl);
	]] 
	-- local baud = '"9600-8N1"'
	local str = string.format('{"whatCvtType":%d,"gpioId":%d,"isInput":%d,"initVal",%d}', whatCvtType, 39, 1, 1)
	return string.len(str), str, delay
end

--[[ MUST
	查询设备状态。
	每个设备都约定需要一条或者多条指令可以获取到设备的所有状态。
]]
function s1GetQueries()
	local query = string.char( 0xab )
	local queryCountLen = string.char( 0x01, 0x00 )
	-- print('hello')
	return string.len( queryCountLen ), queryCountLen, string.len( query ), query
end

--[[ MUST
	WIFI 重置命令判别
]]
function s1GetValidKind(data)
	return 2
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
	local cmd = string.char( 0xcd )
	return string.len(cmd), cmd
end

--[[ MUST
	return value: return 0, 0, if the input param 'bin' is not valid
]]
function s1CvtPri2Std(bin)
	local dataTbl = {}
	local str = '{"switcher":%d}'
	dataTbl = stringToTable(bin)
	-- for i = 1, #bin
	-- 		 (bin[i])
	-- end
	-- LOGTBL(dataTbl)

	str = string.format(str, dataTbl[1])
	return string.len(str), str
end

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
	local gpioId = 39
	local isInput = 1
	--[[
	    LELINK_GPIO_PINMODE_DEFAULT = 0,                      /*!< GPIO pin mode default define */
	    LELINK_GPIO_PINMODE_PULLUP,                          /*!< GPIO pin mode pullup define */
	    LELINK_GPIO_PINMODE_PULLDOWN,                        /*!< GPIO pin mode pulldown define */
	    LELINK_GPIO_PINMODE_NOPULL,                          /*!< GPIO pin mode nopull define */
	    LELINK_GPIO_PINMODE_TRISTATE,                        /*!< GPIO pin mode tristate define */
	]] 
	local gpioMode = 1
	local str = string.format('{"whatCvtType":%d,"gpioId":%d,"isInput":%d,"initVal",%d}', whatCvtType, gpioId, isInput, gpioMode)
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

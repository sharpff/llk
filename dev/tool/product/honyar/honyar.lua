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
	local str = "LUA: "
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
function s1getQueries()
	local query = string.char( 0xa5, 0xa5, 0x5a, 0x5a, 0xb1, 0xc0, 0x01, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00 )
	local queryCountLen = string.char( 0x0d, 0x00 )

	return string.len( queryCountLen ), queryCountLen, string.len( query ), query
end

--[[ MUST
	WIFI 重置命令判别
]]
function s1GetValidKind(data)
    local reset = string.char( 0xA5, 0xA5, 0x5A, 0x5A, 0x98, 0xC1, 0xE8, 0x03, 0x00, 0x00, 0x00, 0x00 )
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

-- 插排
-- a5 a5 5a 5a b1 c0 01 00 03 00 00 00 00       # 查询 01 00
-- a5 a5 5a 5a b9 c0 04 00 04 00 00 00 00 04    # 返回 04 00
-- a5 a5 5a 5a c1 c0 02 00 03 00 00 0f 00       # 控制 02 00
-- a5 a5 5a 5a c8 c0 04 00 04 00 00 00 0f 04    # 返回 04 00
function s1CvtStd2Pri(json)
	local tb = cjson.decode(json)
	local ctrl = tb["ctrl"]
	local cmdTbl = { 0xa5, 0xa5, 0x5a, 0x5a, 0x00, 0x00, 0x02, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00 }
	local dataStr = ""
    local sum = 0xbeaf

    for i = 1, 4 do
    	idx = "idx"..i
    	print(idx, ctrl[idx])
    	if(ctrl[idx] ~= nil) then
			cmdTbl[12] = cmdTbl[12] | (0x01 << (i - 1))
    		if(ctrl[idx] == 1) then
				cmdTbl[13] = cmdTbl[13] | (0x01 << (i - 1))
    		end
    	end
    end
    
    for i = 1, #cmdTbl do
        sum = sum + cmdTbl[i]
    end
    cmdTbl[5] = sum & 0xff
    cmdTbl[6] = (sum >> 8) & 0xff

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
    local str = '{"idx1":%d,"idx2":%d,"idx3":%d,"idx4":%d}'
    -- print (#bin)
    dataTbl = stringToTable(bin)
    LOGTBL(dataTbl)

    if dataTbl[1] == 0xa5 and dataTbl[2] == 0xa5 and dataTbl[3] == 0x5a and dataTbl[4] == 0x5a then
        str = string.format(str, (dataTbl[13] >> 0) & 0x1, (dataTbl[13] >> 1) & 0x1, (dataTbl[13] >> 2) & 0x1, (dataTbl[13] >> 3) & 0x1)
    else
        str = ""
    end
    return string.len(str), str
end

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
	local str = '1.1.1'
	return string.len(str), str
end

--[[ MUST
	whatCvtType:
	0x1. UART json <-> bin
	0x2. GPIO
	0x4. PIPE
	0X8. SOCKET	
]]
function s1GetCvtType()
	local str = [[
    {"whatCvtType":1,
     "common":[{"num":6,"id":"44-45","mux":"2-2"}],
     "uart":[{"id":1, "baud":"9600-8N1"}]
    }
    ]]
	local delay = 5

	return string.len(str), str, delay
end

--[[ MUST
	查询设备状态。
	每个设备都约定需要一条或者多条指令可以获取到设备的所有状态。
]]
function s1GetQueries(queryType)
    local query = ""
    local queryCountLen = ""

    if queryType == 1 then
        query = string.char( 0xbb, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfa, 0x44 )
    end
    if string.len(query) ~= 0 then
        queryCountLen = string.char(string.len(query), 0x00 )
    end

	return string.len( queryCountLen ), queryCountLen, string.len( query ), query
end

--[[ MUST
	WIFI 重置命令判别
]]
function s1GetValidKind(data)
	local reset = string.char(0xa5, 0xa5, 0x5a, 0x5a, 0x98, 0xc1, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00)

	-- print (data, #data)

	--[[ MUST
		wifi reset cmd
	]]
	--print ('start\r\n')
	--tmpTbl = stringToTable(data)
	--LOGTBL(tmpTbl)
	--tmpTbl = stringToTable(reset)
	--LOGTBL(tmpTbl)
	--print (string.find(data, reset))
	--print ('\r\n')
	
	if nil ~= string.find(data, reset) then
		--print '1'
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
	local ctrl = cjson.decode(json)
	local cmdTbl = { 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfa, 0x44 }
	local dataStr = ""

	-- 打开
	if (ctrl["action"] == 1) then
		cmdTbl[2] = 0x01
	-- 关闭
	elseif (ctrl["action"] == 2) then
		cmdTbl[2] = 0x02
	-- 暂停
	elseif (ctrl["action"] == 3) then
		cmdTbl[2] = 0x03
	-- 测量
	elseif (ctrl["action"] == 4) then
		cmdTbl[2] = 0x05
	else
		cmdTbl = {}
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
	local str = '{"percentage":%d}'
	dataTbl = stringToTable(bin)
	-- for i = 1, #bin
	-- 		 (bin[i])
	-- end
	-- LOGTBL(dataTbl)
	if dataTbl[3] > 0 and dataTbl[3] < 20 then
		dataTbl[3] = 10
	elseif dataTbl[3] > 20 and dataTbl[3] < 40 then
		dataTbl[3] = 30
	elseif dataTbl[3] > 40 and dataTbl[3] < 60 then
		dataTbl[3] = 50
	elseif dataTbl[3] > 60 and dataTbl[3] < 80 then
		dataTbl[3] = 70
	elseif dataTbl[3] > 80 and dataTbl[3] < 100 then
		dataTbl[3] = 90
	end
	str = string.format(str, dataTbl[3])
	-- str = string.format(str, #dataTbl)
	-- print (str)
	return string.len(str), str
end
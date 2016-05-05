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

function LOGSTR(sta)
	local str = "LUA: "
	for i = 1, #sta do 
		str = str..string.format('%02x ', sta:byte(i))
	end
	print ('LOGSTR '..str..'\r\n')
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
	local delay = 5
	local baud = '"9600-8N1"'
	local str = string.format('{"whatCvtType":%d,"baud":%s}', whatCvtType, baud)

	return string.len(str), str, delay
end

--[[ MUST
	查询及设备状态指令序列
	每个设备都约定需要一条或者多条指令可以获取到设备的所有状态。
    queryType:
            1, 查询设备状态
            2, 设备进入配置状态
            3, 设备进入连接AP状态
            4, 已经连接到AP，可以本地控制
            5, 已经正常连到云服务，可远程控制
]]
--BB A3 02 00 00 00 00 00 00 00 A5 44             // 查询状态
--A5 A5 5A 5A 98 C1 E8 03 00 00 00 00             // (复位命令)
--A5 A5 5A 5A 99 C1 E9 03 00 00 00 00             // 复位成功后，WIFI模块返回
--网络状态命令：（可以根据实际情况调节改变）.  WIFI模块的状态改变时，会发送以下命令给控制器
--A5 A5 5A 5A A0 C1 EC 03 04 00 00 00 00 00 00 00 //设备进入配置状态（LED快闪）
--A5 A5 5A 5A A1 C1 EC 03 04 00 00 00 01 00 00 00 //设备进入连接AP状态（LED慢闪）
--A5 A5 5A 5A A2 C1 EC 03 04 00 00 00 02 00 00 00 //已经连接到AP，可以本地控制（LED熄灭）
--A5 A5 5A 5A A3 C1 EC 03 04 00 00 00 03 00 00 00 //已经正常连到云服务，可远程控制
function s1GetQueries(queryType)
    --local query = string.char( 0xa5, 0xa5, 0x5a, 0x5a, 0xb1, 0xc0, 0x01, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00 )
    --local queryCountLen = string.char( 0x0d, 0x00 )
    local query = ""
    local queryCountLen = string.char(0x00)

    if queryType == 1 then
        query = string.char(0xbb, 0xa3, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa5, 0x44)
    elseif queryType == 2 then
        query = string.char(0xa5, 0xa5, 0x5a, 0x5a, 0xa0, 0xc1, 0xec, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
    elseif queryType == 3 then
        query = string.char(0xa5, 0xa5, 0x5a, 0x5a, 0xa1, 0xc1, 0xec, 0x03, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00)
    elseif queryType == 4 then
        query = string.char(0xa5, 0xa5, 0x5a, 0x5a, 0xa2, 0xc1, 0xec, 0x03, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00)
    elseif queryType == 5 then
        query = string.char(0xa5, 0xa5, 0x5a, 0x5a, 0xa3, 0xc1, 0xec, 0x03, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00)
    end
    if string.len(query) ~= 0 then
        queryCountLen = string.char(string.len(query), 0x00 )
    end

    --print(string.len(queryCountLen), string.len( query ), "\n")
    --LOGSTR(queryCountLen)
    --LOGSTR(query)
    return string.len( queryCountLen ), queryCountLen, string.len( query ), query
end

--[[ MUST
	WIFI 重置命令判别
]]
function s1GetValidKind(data)
    local reset = string.char( 0xa5, 0xa5, 0x5a, 0x5a, 0x98, 0xc1, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00)

	if nil ~= string.find(data, reset) then
		return 1
	end

	if 12 == #data then
		return 2
	end

	return 0
end

-- 晾霸
--{ "ctrl": { "action":0, "speaker":0, "anti":0, "anion":0, "h-dry":0, "w-dry":0, "light":1, "wDryTime":0, "hDryTime":0, "antiTime":0 } }
--"{ \"ctrl\": { \"action\":0, \"speaker\":0, \"anti\":0, \"anion\":0, \"h-dry\":0, \"w-dry\":0, \"light\":1, \"wDryTime\":0, \"hDryTime\":0, \"antiTime\":0 } }"
--"{ \"ctrl\": { \"action\":0, \"speaker\":0, \"anti\":0, \"anion\":0, \"h-dry\":0, \"w-dry\":0, \"light\":0, \"wDryTime\":0, \"hDryTime\":0, \"antiTime\":0 } }"
function s1CvtStd2Pri(json)
	local tb = cjson.decode(json)
	local ctrl = tb["ctrl"]
	local cmdTbl = { 0xbb, 0xa3, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44}
	local dataStr = ""
    local sum = 0

    cmdTbl[4] = cmdTbl[4] | (ctrl["action"] & 0x3 << 6)
    cmdTbl[4] = cmdTbl[4] | (ctrl["speaker"] & 0x1 << 5)
    cmdTbl[4] = cmdTbl[4] | (ctrl["anti"] & 0x1 << 4)
    cmdTbl[4] = cmdTbl[4] | (ctrl["anion"] & 0x1 << 3)
    cmdTbl[4] = cmdTbl[4] | (ctrl["h-dry"] & 0x1 << 2)
    cmdTbl[4] = cmdTbl[4] | (ctrl["w-dry"] & 0x1 << 1)
    cmdTbl[4] = cmdTbl[4] | (ctrl["light"] & 0x1 << 0)
    
    for i = 2, #cmdTbl - 3 do
        sum = sum + cmdTbl[i]
    end
    cmdTbl[10] = (sum >> 8) & 0xff
    cmdTbl[11] = sum & 0xff

	--LOGTBL(cmdTbl)
	dataStr = tableToString(cmdTbl)
    --LOGSTR(dataStr);
	return string.len(dataStr), dataStr
end

--[[ MUST
	return value: return 0, 0, if the input param 'bin' is not valid
]]
--bb a3 03 01 00 00 00 00 00 00 a7 44
function s1CvtPri2Std(bin)
    local str = ""
    local dataTbl = {}
    local fmtstr = "{ \"action\":%d, \"speaker\":%d, \"anti\":%d, \"anion\":%d, \"h-dry\":%d, \"w-dry\":%d, \"light\":%d, \"wDryTime\":%d, \"hDryTime\":%d, \"antiTime\":%d, \"pos\":%d } }"

    dataTbl = stringToTable(bin)
    --LOGTBL(dataTbl)
    if #dataTbl >= 12 and dataTbl[1] == 0xbb and dataTbl[2] == 0xa3 and dataTbl[3] == 0x03 then
        local sum = 0
        local chsum = (dataTbl[10] << 8 | dataTbl[11])
        for i = 2, #dataTbl - 3 do
            sum = sum + dataTbl[i]
        end
        if sum == chsum then
            local cmd = dataTbl[4]
            str = string.format(fmtstr, (cmd >> 6 & 0x3), (cmd >> 5 & 0x1), (cmd >> 4 & 0x1), (cmd >> 3 & 0x1), (cmd >> 2 & 0x1), (cmd >> 1 & 0x1), (cmd & 0x1), 
            dataTbl[5], dataTbl[6], dataTbl[7], dataTbl[8])
        end
    end

    --print(str)
    return string.len(str), str
end


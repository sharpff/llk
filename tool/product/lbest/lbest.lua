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
	local str = '1.2.3'
	return string.len(str), str
end

--[[ MUST
	whatCvtType:
	0. UART json <-> bin
	1. PIPE/IPC json <-> json
]]
function s1GetCvtType()

    local str = [[
    {"whatCvtType":1,
     "common":[{"num":2,"id":"2-3","mux":"7-7"}],
     "uart":[{"id":0, "baud":"9600-8N1"}]
    }
    ]]
    local delay = 5
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

--V1.0
--
--A5 A5 5A 5A 98 C1 E8 03 00 00 00 00             // (复位命令)
--A5 A5 5A 5A 99 C1 E9 03 00 00 00 00             // 复位成功后，WIFI模块返回
--A5 A5 5A 5A A0 C1 EC 03 04 00 00 00 00 00 00 00 //设备进入配置状态（LED快闪）
--A5 A5 5A 5A A1 C1 EC 03 04 00 00 00 01 00 00 00 //设备进入连接AP状态（LED慢闪）
--A5 A5 5A 5A A2 C1 EC 03 04 00 00 00 02 00 00 00 //已经连接到AP，可以本地控制（LED熄灭）
--A5 A5 5A 5A A3 C1 EC 03 04 00 00 00 03 00 00 00 //已经正常连到云服务，可远程控制

--V1.3
--
--BB A3 04 00 00 00 00 00 00 00 A7 44             // (复位命令)
--BB A3 07 00 00 00 00 00 00 00 AA 44             // 复位成功后，WIFI模块返回
--BB A3 07 00 00 00 00 00 00 00 AA 44             //设备进入配置状态（LED快闪)
--BB A3 08 00 00 00 00 00 00 00 AB 44             //设备进入连接AP状态（LED慢闪）
--BB A3 09 00 00 00 00 00 00 00 AC 44             //已经连接到AP，可以本地控制（LED熄灭）
--BB A3 0A 00 00 00 00 00 00 00 AD 44             //已经正常连到云服务，可远程控制

function s1GetQueries(queryType)
    --local query = string.char( 0xa5, 0xa5, 0x5a, 0x5a, 0xb1, 0xc0, 0x01, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00 )
    --local queryCountLen = string.char( 0x0d, 0x00 )
    local query = ""
    local queryCountLen = string.char(0x00)

    if queryType == 1 then
        query = string.char(0xbb, 0xa3, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa5, 0x44)
    elseif queryType == 2 then
        query = string.char(0xBB, 0xA3, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA, 0x44)
    elseif queryType == 3 then
        query = string.char(0xBB, 0xA3, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAB, 0x44)
    elseif queryType == 4 then
        query = string.char(0xBB, 0xA3, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAC, 0x44)
    elseif queryType == 5 then
        query = string.char(0xBB, 0xA3, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAD, 0x44)
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
    local reset = string.char(0xBB, 0xA3, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA7, 0x44)

	if nil ~= string.find(data, reset) then
		return 1
	end

	if 12 == #data then
		return 2
	end

	return 0
end

-- 晾霸
function s1CvtStd2Pri(json)
    local sum = 0
	local dataStr = ""
	local cmdTbl = { 0xbb, 0xa3, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44}
	local ctrl = cjson.decode(json)
    local stalen, stastr = s1apiGetDevStatus()

	print ('STASTR: '..stastr..'\n')
    local status = cjson.decode(stastr)
	if not ctrl["action"] then 
		ctrl["action"] = status["action"]
	end
	if not ctrl["speaker"] then 
		ctrl["speaker"] = status["speaker"]
	end
	if not ctrl["anion"] then 
		ctrl["anion"] = status["anion"]
	end
	if not ctrl["light"] then 
		ctrl["light"] = status["light"]
	end
	if ctrl["disinfect"] then 
        if ctrl["disinfect"] ~= 1 then
            ctrl["DTS"] = 0 
        end
	else
		ctrl["disinfect"] = status["disinfect"]
        ctrl["DTS"] = status["DTL"]
	end
	if ctrl["bake"] then 
        if ctrl["bake"] ~= 1 then 
            ctrl["BTS"] = 0
        end
	else
		ctrl["bake"] = status["bake"]
        ctrl["BTS"] = status["BTL"]
	end
	if ctrl["wind"] then 
        if ctrl["wind"] ~= 1 then 
            ctrl["WTS"] = 0
        end
	else
		ctrl["wind"] = status["wind"]
        ctrl["WTS"] = status["WTL"]
	end

    cmdTbl[4] = cmdTbl[4] | ((ctrl["action"] & 0x3 ) << 6)
    cmdTbl[4] = cmdTbl[4] | ((ctrl["speaker"] & 0x1 ) << 5)
    cmdTbl[4] = cmdTbl[4] | ((ctrl["disinfect"] &0x1 ) << 4)
    cmdTbl[4] = cmdTbl[4] | ((ctrl["anion"] & 0x1 ) << 3)
    cmdTbl[4] = cmdTbl[4] | ((ctrl["bake"] & 0x1 ) << 2)
    cmdTbl[4] = cmdTbl[4] | ((ctrl["wind"] & 0x1 ) << 1)
    cmdTbl[4] = cmdTbl[4] | ((ctrl["light"] & 0x1 ) << 0)
    cmdTbl[5] = ctrl["WTS"]
    cmdTbl[6] = ctrl["BTS"]
    cmdTbl[7] = ctrl["DTS"]

    for i = 2, #cmdTbl - 3 do
        sum = sum + cmdTbl[i]
    end
    cmdTbl[10] = (sum >> 8) & 0xff
    cmdTbl[11] = sum & 0xff

    LOGTBL(cmdTbl)
	dataStr = tableToString(cmdTbl)
    LOGSTR(dataStr);
	return string.len(dataStr), dataStr
end

--[[ MUST
	return value: return 0, 0, if the input param 'bin' is not valid
]]
--bb a3 03 01 00 00 00 00 00 00 a7 44
function s1CvtPri2Std(bin)
    local str = ""
    local dataTbl = {}
    local fmtstr = "{\"action\":%d, \"speaker\":%d, \"disinfect\":%d, \"anion\":%d, \"bake\":%d, \"wind\":%d, \"light\":%d, \"WTL\":%d, \"BTL\":%d, \"DTL\":%d, \"pos\":%d}"

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


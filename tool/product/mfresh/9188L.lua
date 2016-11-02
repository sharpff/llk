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
	local str = "[LUA] "
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
	local str = '1.0.1'
	return string.len(str), str
end

--[[ MUST
	whatCvtType:
	0x1. UART json <-> bin
	0x2. GPIO
	0x4. PIPE
	0x8. SOCKET
    0x10.PWM
]]
function s1GetCvtType()
    -- combained uart(0x1) & gpio(0x2) & pwm(0x10)
    local str = [[
    {"whatCvtType":1,
     "common":[{"num":2,"id":"36-37","mux":"7-7"}],
     "uart":[{"id":1, "baud":"115200-8N1"}]
    }
    ]]
    local delay = 5
    return string.len(str), str, delay
end

--[[ MUST
	查询设备状态。
	每个设备都约定需要一条或者多条指令可以获取到设备的所有状态。
]]
-- {0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x42}
function s1GetQueries(queryType)
    local query = ""
    local queryCountLen = ""
    local cvtType = s1apiGetCurrCvtType()
    if cvtType == 1 then
	    if queryType == 1 then
	        query = string.char(0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x42)
	    end
	    if queryType == 2 or queryType == 3 then
	        query = string.char(0x42,0x15,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x57)
	    end
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
	local cvtType = s1apiGetCurrCvtType()
	local dataTbl = nil
	local tmp = stringToTable(data)
	if 0x01 == cvtType then
		dataTbl = stringToTable(data)
		if 0x14 == dataTbl[2] then
			return 1
		end
	end
	return 2
end

--[[ MUST
]]
-- {"ctrl":{"pwr":1,"speed":4,"os-pm2.5":276,"reset-time":1}}
-- 开，关，设置风速模式(1,2,3,智能,夜间,喷射)，重置滤网，室外PM2.5
-- {0x42,0x00,0x03,0x31,0x00,0x50,0x00,0x00,0x00,0x00,0xA1,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x27,0x00,0x00,0x1C,0x11,0xC0}
-- {0x42,0x00,0x03,0x30,0x00,0x50,0x00,0x00,0x00,0x00,0xA1,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x27,0x00,0x00,0x1C,0x11,0xBF}

-- {0x42,0x00,0x05,0x31,0x00,0x51,0x00,0x00,0x00,0x00,0xA1,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x27,0x00,0x00,0x1C,0x11,0xC3}
-- {0x42,0x00,0x05,0x31,0x00,0x52,0x00,0x00,0x00,0x00,0xA1,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x27,0x00,0x00,0x1C,0x11,0xC4}
-- {0x42,0x00,0x05,0x31,0x00,0x53,0x00,0x00,0x00,0x00,0xA1,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x27,0x00,0x00,0x1C,0x11,0xC5}
-- {0x42,0x00,0x05,0x31,0x00,0x55,0x00,0x00,0x00,0x00,0xA1,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x27,0x00,0x00,0x1C,0x11,0xC7}
-- {0x42,0x00,0x05,0x31,0x00,0x50,0x00,0x00,0x00,0x00,0xA1,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x27,0x00,0x00,0x1C,0x11,0xC2}
-- {0x42,0x00,0x05,0x31,0x00,0x54,0x00,0x00,0x00,0x00,0xA1,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x27,0x00,0x00,0x1C,0x11,0xC6}

-- {0x42,0x00,0x0D,0x31,0x00,0x50,0x00,0x00,0x00,0x00,0xA1,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x27,0x00,0x00,0x1C,0x11,0xCB}
-- {0x42,0x00,0x0E,0x31,0x00,0x50,0x00,0x00,0x00,0x00,0xA1,0x00,0x00,0x00,0x05,0x01,0x00,0x00,0x00,0x27,0x00,0x00,0x1C,0x11,0xCD}
function s1CvtStd2Pri(json)
    local sum = 0
    local count = 0
    local cmdtb = {0x42,0x00,0x03,0x31,0x00,0x50,0x00,0x00,0x00,0x00,0xA1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x27,0x00,0x00,0x1C,0x11,0xC0}
    local ctrl = cjson.decode(json)
    local cvtType = s1apiGetCurrCvtType()
    -- cvtType = 1
    if cvtType == 1 then
        local pwr = ctrl["pwr"]
		if pwr == 0 then
			cmdtb[4] = 0x30
		end
		local speed = ctrl["speed"]
		if speed ~= nil then
			if speed == 1 then
				cmdtb[3] = 0x05
				cmdtb[6] = 0x50
			elseif speed == 2 then
				cmdtb[3] = 0x05
				cmdtb[6] = 0x51
			elseif speed == 3 then
				cmdtb[3] = 0x05
				cmdtb[6] = 0x52
			elseif speed == 4 then
				cmdtb[3] = 0x05
				cmdtb[6] = 0x53
			elseif speed == 5 then
				cmdtb[3] = 0x05
				cmdtb[6] = 0x54
			elseif speed == 6 then
				cmdtb[3] = 0x05
				cmdtb[6] = 0x55
			end
	    end
		local reset = ctrl["reset-time"]
		if reset ~= nil then
			cmdtb[3] = 0x0D
		end
		local ospm2 = ctrl["outdoor_pm25"]
		if ospm2 ~= nil then
			for i=1, #cmdtb do
				cmdtb[i] = cmdtb[i]
				print('out => ' .. cmdtb[i] ..'\n')
			end
			cmdtb[3] = 0x0E
			cmdtb[15] = ospm2 & 0xff
			cmdtb[16] = (ospm2 >> 8) & 0xff
			print('aaaaa => ' .. ospm2..' '..cmdtb[15]..' '..cmdtb[16]..'\n')
		else
			print('bbbbb' .. '\n')
		end
		for i = 1, #cmdtb - 1 do
			sum = sum + cmdtb[i]
		end
		cmdtb[25] = sum & 0xff
		count = 25
	end
    LOGTBL(cmdtb)
    local cmd = tableToString(cmdtb)
    return count, cmd
end

--[[ MUST
	return value: return 0, 0, if the input param 'bin' is not valid
]]
function s1CvtPri2Std(bin)
    local dataTbl = {}
	local sum = 0
	local str = ""
	local lastData = 0
    local status = '{"pwr":%d, "envtemp":%d, "humidity":%d, "indoor_pm25":%d, "mode":%d, "used":%d}'
    local cvtType = s1apiGetCurrCvtType()
    local lenStatus, currStatus = s1apiGetDevStatus()
    if lenStatus <= 2 then
        lastData = 0
    else
    	local tb = cjson.decode(currStatus)
    	lastData = tb["indoor_pm25"]
    end
    if cvtType == 1 then
        dataTbl = stringToTable(bin)
		for i = 1, #dataTbl - 1 do
			sum = sum + dataTbl[i]
		end
	    sum = sum & 0xff
		if sum == dataTbl[25] then
			local pwd = dataTbl[4] - 0x30
			local envtemp = dataTbl[18]
			local humidity = dataTbl[20]
			local mode = dataTbl[6] - 0x4F
			local pm = dataTbl[16]
			pm = (pm << 8) | dataTbl[15]
			if lastData == nil then
				lastData = 0
			end
			local temp = pm - lastData
			if temp >= 0 and temp < 5 then
				pm = lastData
			end
			if temp < 0 and temp > -5 then
				pm = lastData
			end
			local used = dataTbl[24]
			used = (used << 8) | dataTbl[23]
			if dataTbl[17] == 1 then
				envtemp = envtemp * (-1)
			end
			str = string.format(status, pwd, envtemp, humidity, pm, mode, used)
		end
    end
    return string.len(str), str
end


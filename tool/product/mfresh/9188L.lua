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
	local str = '2.0.3'
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
    -- combained uart(0x1)
    local str = [[
    {"whatCvtType":1,
     "common":[{"num":2,"id":"2-3","mux":"7-7"}],
     "uart":[{"id":0, "baud":"115200-8N1"}]
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
	    if queryType >= 3 or queryType == 1 then
	        query = string.char(0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x42)
	    end
	    if queryType == 2 then
	         query = string.char(0x42,0x15,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x57)
	    end
	end
    if string.len(query) ~= 0 then
        queryCountLen = string.char(string.len(query), 0x00 )
    end
    return string.len( queryCountLen ), queryCountLen, string.len( query ), query
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
function s1apiGetDevStatus( ... )
	local status = '{"pwr":0, "envtemp":21, "humidity":14, "indoor_pm25":36, "mode":2, "used":3409}'
	return string.len(status), status
end

function s1apiGetCurrCvtType( ... )
	return 1
end

function genPriDataFormat(items, ioType)
    local r = {}
    local k = 1
	for i = 1, #items do
		r[k] = ioType -- io type
		k = k + 1
		r[k] = #items[i] -- current item len
		k = k + 1

		-- -- CUST YOUR CONFIG START
		-- r[k] = 0x05 -- ack wait ms (little-endian L)
		-- k = k + 1
		-- r[k] = 0x00 -- ack wait ms (little-endian H)
		-- k = k + 1
		-- r[k] = 0x30 -- write delay ms (little-endian L)
		-- k = k + 1
		-- r[k] = 0x00 -- write delay ms (little-endian H)
		-- k = k + 1
		-- r[k] = 0x00 -- reserved
		-- k = k + 1
		-- r[k] = 0x00 -- reserved
		-- k = k + 1
		-- -- CUST YOUR CONFIG END

		for j = 1, #items[i] do
			r[k] = items[i][j]
			-- print(string.format('%02x \r\n', items[i][j]))
			k = k + 1
		end
	end
    return r
end

function s1CvtStd2Pri(json)
    local sum = 0
    local count = 0
    -- local offset = 0
    local item = 1
    local ctrl = cjson.decode(json)
    local cvtType = s1apiGetCurrCvtType()
    local lenStatus, currStatus = s1apiGetDevStatus()
    local cmdtb = {}
    if cvtType == 1 then
    	cmdtb = {{0x42,0x00,0x03,0x31,0x00,0x50,0x00,0x00,0x00,0x00,0xA1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x27,0x00,0x00,0x1C,0x11,0xBB},
					{0x42,0x00,0x03,0x31,0x00,0x50,0x00,0x00,0x00,0x00,0xA1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x27,0x00,0x00,0x1C,0x11,0xC0}}
        local pwr = ctrl["pwr"]
        local speed = ctrl["speed"]
		if pwr == 0 then
			cmdtb[item][4] = 0x30
			-- print('mmmmmmmmmmmm6\r\n')
		else
			local tb = cjson.decode(currStatus)
			local pwr_old = tb["pwr"]
		
			-- print(string.format('====> %d %d\r\n', pwr_old, speed))
			if pwr_old == 0 and speed ~= nil and speed ~= 6 then
				-- offset = 25
				-- print('mmmmmmmmmmmm4\r\n')
				item = 2
			end
		end
		if speed ~= nil then
			if speed == 1 then
				cmdtb[item][3] = 0x05
				cmdtb[item][6] = 0x50
			elseif speed == 2 then
				cmdtb[item][3] = 0x05
				cmdtb[item][6] = 0x51
			elseif speed == 3 then
				cmdtb[item][3] = 0x05
				cmdtb[item][6] = 0x52
			elseif speed == 4 then
				cmdtb[item][3] = 0x05
				cmdtb[item][6] = 0x53
			elseif speed == 5 then
				cmdtb[item][3] = 0x05
				cmdtb[item][6] = 0x54
			elseif speed == 6 then
				cmdtb[item][3] = 0x05
				cmdtb[item][6] = 0x55
			end
	    end
		local reset = ctrl["reset-time"]
		if reset ~= nil then
			cmdtb[item][3] = 0x0D
		end
		local ospm2 = ctrl["outdoor_pm25"]
		if ospm2 ~= nil then
			-- for i=1, #cmdtb do
			-- 	cmdtb[i] = cmdtb[i]
			-- end
			cmdtb[item][3] = 0x0E
			cmdtb[item][15] = ospm2 & 0xff
			cmdtb[item][16] = (ospm2 >> 8) & 0xff
		end
		-- if offset == 0 then
		-- 	for i = 3, #cmdtb - 28 do
		-- 		sum = sum + cmdtb[i]
		-- 	end
		-- 	cmdtb[27] = sum & 0xff
		-- 	count = 27
		-- else
		-- 	for i = 30, #cmdtb - 1 do
		-- 		sum = sum + cmdtb[i]
		-- 	end
		-- 	cmdtb[54] = sum & 0xff
		-- 	count = 54
		--    end
	 	for j = 1, item do
	 		sum = 0
			for i = 1, #cmdtb[j] - 1 do
				sum = sum + cmdtb[j][i]
			end
			cmdtb[j][#cmdtb[j]] = sum & 0xff
		end
		local tmp = {}
		if item == 1 then
			tmp[1] = cmdtb[1]
		else
			tmp = cmdtb
		end
		cmdtb = genPriDataFormat(tmp, cvtType)
		-- for l = 1, #cmdtb do
			-- a[l] = a[l]
			-- print(string.format('aa %02x \r\n', a[l]))
		-- end
	end
    LOGTBL(cmdtb)
    local cmd = tableToString(cmdtb)
    return #cmd, cmd
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
		if #dataTbl > 27 then
            if 0x14 == dataTbl[27] then
				s1apiRebootDevice()
			end
        end
		for i = 1, 24 do
			sum = sum + dataTbl[i]
		end
	    sum = sum & 0xff
		if sum == dataTbl[25] then
			local pwd = dataTbl[4] - 0x30
			local envtemp = dataTbl[18]
			local humidity = dataTbl[20]
			local mode = dataTbl[6] - 0x4F
			local pm = dataTbl[16]
			if 0x14 == dataTbl[2] then
				s1apiRebootDevice()
			end
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


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
	local str = '1.0.0'
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
    {"whatCvtType":19,
     "common":[{"num":8,"id":"36-37-32-33-34-35-0","mux":"7-7-9-9-9-9-8"}],
     "uart":[{"id":1, "baud":"9600-8N1"}],  
     "gpio":[{"id":0,"dir":0,"mode":0,"state":1,"type":1,"longTime":30,"shortTime":3}],
     "pwm":[{"id":33,"type":0,"clock":1,"state":1024,"frequency":5120,"duty":1024},
            {"id":34,"type":0,"clock":1,"state":1024,"frequency":5120,"duty":1024},
            {"id":35,"type":0,"clock":1,"state":1024,"frequency":5120,"duty":1024},
            {"id":18,"type":0,"clock":1,"state":1024,"frequency":5120,"duty":1024}]
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
    local cvtType = s1apiGetCurrCvtType()
    if queryType == 1 and cvtType == 1 then
        query = string.char( 0xab )
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
	return 2
end

--[[ MUST
]]
-- type, size, id, val, id, val ...
-- {"pwm":[{"id":%d,"val":%d},{"id":%d,"val":%d},{"id":%d,"val":%d},{"id":%d,"val":%d}]}
function s1CvtStd2Pri(json)
    local i = 0, val
    local j = 0
    local cmdtb = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    local tb = cjson.decode(json)
    local cvtType = s1apiGetCurrCvtType()
    if cvtType == 16 then
        local ctrl = tb["pwm"]
        for j = 1, 4 do
	        val = ctrl[j]["id"]
            i = i + 1
            cmdtb[i] = val
            val = ctrl[j]["val"]
            i = i + 1
            cmdtb[i] = (val >> 8) & 0xff
            i = i + 1
            cmdtb[i] = val & 0xFF
        end
    end
    local cmd = tableToString(cmdtb)
    return i, cmd
end

--[[ MUST
	return value: return 0, 0, if the input param 'bin' is not valid
]]
function s1CvtPri2Std(bin)
    local i = 0, val, j
    local len = 0
    local str = ""
    local datatb = {33, 0, 34, 0, 35, 0, 18, 0}
    local status = '{"pwm":[{"id":%d,"val":%d},{"id":%d,"val":%d},{"id":%d,"val":%d},{"id":%d,"val":%d}]}'
    local cvtType = s1apiGetCurrCvtType()
    local lenStatus, currStatus = s1apiGetDevStatus()
    if lenStatus <= 2 then
        str = string.format(status, 33, 0, 34, 0, 35, 0, 18, 0)
    end
    if cvtType == 2 then
        local ctrltb = {}
        local id = bin:byte(1)
        if id == 0 and bin:byte(2) == 1 then
            if lenStatus > 2 then
    	        local tb = cjson.decode(currStatus)
    	        local pwm = tb["pwm"]
    	        for j = 1, 4 do
        		    val = pwm[j]["id"]
        		    i = i + 1
        		    datatb[i] = val
        		    val = pwm[j]["val"]
        		    i = i + 1
        		    datatb[i] = val
    	        end
            end
            if datatb[8] == 0 then
                str = string.format(status, datatb[1], 1024, datatb[3], 1024, datatb[5], 1024, datatb[7], 1024)
            else
                str = string.format(status, datatb[1], 0, datatb[3], 0, datatb[5], 0, datatb[7], 0)
            end
        end
        len = 0x40000000
    end
    if cvtType == 16 and #bin >= 12 then
        local id1, id2, id3, id4, val1, val2, val3, val4
        id1 = bin:byte(1)
        val1 = bin:byte(2)
        val1 = (val1 << 8) | bin:byte(3)
        id2 = bin:byte(4)
        val2 = bin:byte(5)
        val2 = (val2 << 8) | bin:byte(6)      
        id3 = bin:byte(7)
        val3 = bin:byte(8)
        val3 = (val3 << 8) | bin:byte(9)         
        id4 = bin:byte(10)
        val4 = bin:byte(11)
        val4 = (val4 << 8) | bin:byte(12) 
        str = string.format(status, id1, val1, id2, val2, id3, val3, id4, val4)
    end
    return  string.len(str) + len, str
end

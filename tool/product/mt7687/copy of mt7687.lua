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
	local str = '1.1'
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
    {
    "whatCvtType":19,
    "common":[
        {    
             "num":7,
             "id":"36-37-32-33-34-35-0",
             "mux":"7-7-9-9-9-9-8"
        }
        ]
    "uart":[
    	{
            "id":1, 
    	    "baud":"9600-8N1"
    	}
    	]   
    "gpio":[
        {
            "id":0,
            "dir":0,
            "mode":1,
            "state":0,
            "type":1,
            "longTime":30,
            "shortTime":3
        }
        ]
     "pwm":[
        {
            "id":33,
            "type":0,
            "clock":1,
            "frequency":1024,
            "duty":255,
        },
        {
            "id":34,
            "type":0
            "clock":1,
            "frequency":1024,
            "duty":255,
        },
        {
            "id":35,
            "type":0
            "clock":1,
            "frequency":1024,
            "duty":255,
        },
        {
            "id":18,
            "type":1,
            "clock":1,
            "frequency":1024,
            "duty":255,
            "blink":2,
            "longTime":4,
            "shortTime":2
        }
        ]
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
-- {"ctrl":{"blink":0, "r":(0-255), "g":(0-255), "b":(0-255), "a":(0-255)}}, 0 - noblink; 1 - blink;
function s1CvtStd2Pri(json)
    local i = 0, val
    local cmdtb = {0, 0, 0, 0, 0}
    local tb = cjson.decode(json)
    local ctrl = tb["ctrl"]
    val = ctrl["blink"]
    if val ~= nil and type(val) == "number" then
        i = i + 1
        cmdtb[i] = (val & 0xFF)
    end
    val = ctrl["r"]
    if val ~= nil and type(val) == "number" then
        i = i + 1
        cmdtb[i] = (val & 0xFF)
    end
    val = ctrl["g"]
    if val ~= nil and type(val) == "number" then
        i = i + 1
        cmdtb[i] = (val & 0xFF)
    end
    val = ctrl["b"]
    if val ~= nil and type(val) == "number" then
        i = i + 1
        cmdtb[i] = (val & 0xFF)
    end
    val = ctrl["a"]
    if val ~= nil and type(val) == "number" then
        i = i + 1
        cmdtb[i] = (val & 0xFF)
    end
    local cmd = tableToString(cmdtb)
	return i, cmd
end

--[[ MUST
	return value: return 0, 0, if the input param 'bin' is not valid
]]
function s1CvtPri2Std(bin)
    local str = ""
    if bin:byte(2) == 0x02 then
    local ctrltb = {}
	for i=2, #bin do
        local val = bin:byte(i) & 0xFF
        if i == 2 then
            ctrltb["blink"] = val
        elseif i == 3 then
            ctrltb["r"] = val
        elseif i == 4 then
            ctrltb["g"] = val
        elseif i == 5 then
            ctrltb["b"] = val
        elseif i == 6 then
            ctrltb["a"] = val
        end
	end
    str = cjson.encode(ctrltb)
    end
	return string.len(str), str
end



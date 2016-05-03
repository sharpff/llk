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

--local conf = [[
--{
--    "whatCvtType": 1,                   # 0 - uart; 1 - gpio;
--    "conf":
--    {
--        "freq": "9600-8N1",             # frequency
--        "bits": 8,                      # data bits
--        "parity": 0,                    # 0 - none; 1 - odd; 2 - even
--        "stopbits": 1,                  # stop bits
--    }
--    或者
--    {
--        # key
--        {
--            "id": 1,                        # support 1, 2, 3
--            "dir": 0,                       # 0 - input; 1 - output
--            "mode": 2,                      # 0 - default; 1 - pullup; 2 - pulldown; 3 - nopull; 4 - tristate
--            "blink": 6,                     # only output. tick, blink frequency.
--            "type":1,                       # 0 - stdio; 1 - reset output/input
--            "state": 0,                     # init state. 0 - low; 1 - high; 2 - blink
--            // for input type 1 (reset)
--            "longTime":30,
--            "shortTime":3,
--        },
--        # led, reset output
--        {
--            "id":2,
--            "dir":1,
--            "mode":0,
--            "state":1,
--            "blink":2,
--            "type":1,
--            // for output type 1 (reset)
--            "longTime":10,
--            "shortTime":1,
--        },
--        # supply hub
--        {
--            "id":3,
--            "dir":1,
--            "mode":0,
--            "state":0
--            "blink":3,
--            "type":0,
--        }
--   }
--]]
function s1GetCvtType()
    --key, led, hub
    local str = [[
    {
        "whatCvtType": 1,
        "conf":
        [
            {
                "id":1,
                "dir":0,
                "mode":2,
                "type":1,

                "longTime":30,
                "shortTime":3,
            },
            {
                "id":2,
                "dir":1,
                "mode":0,
                "state":1,
                "blink":2,
                "type":1,

                "longTime":10,
                "shortTime":1,
            },
            {
                "id":3,
                "dir":1,
                "mode":0,
                "state":0
                "blink":30,

                "type":0,
            }
        ]
    }
    ]]
	return string.len(str), str, 20
end

--[[ MUST
	查询设备状态。
	每个设备都约定需要一条或者多条指令可以获取到设备的所有状态。
]]
function s1GetQueries()
	return 0, "", 0, ""
end

--[[ MUST
	WIFI 重置命令判别
]]
function s1GetValidKind(data)
	return 2
end

--[[ MUST
]]
-- {"ctrl":{"led":2, "hub":1}}, 0 - low; 1 - high; 2 - blink
function s1CvtStd2Pri(json)
    local i = 0, val
    local cmdtb = {0, 0, 0}
	local tb = cjson.decode(json)
	local ctrl = tb["ctrl"]
    val = ctrl["key"]
    if val ~= nil and type(val) == "number" then
        i = i + 1
        cmdtb[i] = (1 << 4) | (val & 0xF)
    end
    val = ctrl["led"]
    if val ~= nil and type(val) == "number" then
        i = i + 1
        cmdtb[i] = (2 << 4) | (val & 0xF)
    end
    val = ctrl["hub"]
    if val ~= nil and type(val) == "number" then
        i = i + 1
        cmdtb[i] = (3 << 4) | (val & 0xF)
    end
    local cmd = tableToString(cmdtb)
	return i, cmd
end

--[[ MUST
	return value: return 0, 0, if the input param 'bin' is not valid
]]
function s1CvtPri2Std(bin)
    local str
    local ctrltb = {}
	for i=1, #bin do
        local id = (bin:byte(i) >> 4) & 0xF
        local val = bin:byte(i) & 0xF
        if id == 1 then
            ctrltb["key"] = val
        elseif id == 2 then
            ctrltb["led"] = val
        elseif id == 3 then
            ctrltb["hub"] = val
        end
	end
    str = cjson.encode(ctrltb)
	return string.len(str), str
end


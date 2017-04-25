--[[ 
	THIS IS FW SCRIPT for simu.lua
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
	local str = '2.0.0'
	return string.len(str), str
end

--[[ MUST
	whatCvtType:
	0. UART json <-> bin
	1. PIPE/IPC json <-> json
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
	return 2
end

--[[ MUST
	make sure s1GetVer is '2.x.x'
]]
function genPriDataFormat(items, ioType)
    local r = {}
    local k = 1
	for i = 1, #items do
		r[#r+1] = ioType -- io type
		r[#r+1] = #items[i] -- current item len

		-- -- CUST YOUR CONFIG START
		r[#r+1] = 0xff -- ack wait ms (little-endian L)
		r[#r+1] = 0x00 -- ack wait ms (little-endian H)
		r[#r+1] = 0x00 -- write delay ms (little-endian L)
		r[#r+1] = 0x00 -- write delay ms (little-endian H)
		r[#r+1] = 0x00 -- reserved
		r[#r+1] = 0x00 -- reserved
		-- -- CUST YOUR CONFIG END

		for j = 1, #items[i] do
			r[#r+1] = items[i][j]
		end
	end

    return r
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
	cmdTbl = {stringToTable('{"test":1}'),
		stringToTable('{"function":1}')}

	local dataStr = ""

	cmdTbl = genPriDataFormat(cmdTbl, 1) -- in PIPE case ioType val should be 4
	LOGTBL(cmdTbl)

	-- u have to make the bin as string for the return value
	dataStr = tableToString(cmdTbl)
	return string.len(dataStr), dataStr
end

--[[ MUST
	return value: return 0, 0, if the input param 'bin' is not valid
]]
function s1CvtPri2Std(bin)
	-- presume data
	bin = '{"result":1}'
	str = bin
	print (str..'\r\n')
	return string.len(str), str
end

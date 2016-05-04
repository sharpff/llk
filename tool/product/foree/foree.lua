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

--[[ MUST
	whatCvtType:
	0. UART json <-> bin
	1. PIPE/IPC json <-> json
]]
function s1GetCvtType()
    -- combained uart(0x1) & gpio(0x2)
    local str = [[
    {
    "whatCvtType":3,
    "uart":[
    	{
    		"id":1, 
    		"baud":"115200-8N1"
    	}
    ],
    "gpio":[
	        {
	            "id":1,
	            "dir":0,
	            "mode":2,
	            "type":1,
	            "longTime":30,
	            "shortTime":3
	        },
	        {
	            "id":2,
	            "dir":1,
	            "mode":0,
	            "state":1,
	            "blink":2,
	            "type":1,
	            "longTime":10,
	            "shortTime":1
	        },
	        {
	            "id":3,
	            "dir":1,
	            "mode":0,
	            "state":0,
	            "blink":30,
	            "type":0
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
	local cvtType = s1apiGetCurrCvtType()
	-- print ('s1GetQueries return => '..cvtType..'\r\n')
	local query = string.char( 0xbb, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfa, 0x44 )
	local queryCountLen = string.char( 0x09, 0x00 )

	return string.len( queryCountLen ), queryCountLen, string.len( query ), query
end

--[[ MUST
	 
]]
function s1GetValidKind(data)
	local cvtType = s1apiGetCurrCvtType()
	-- print ('s1GetValidKind return => '..cvtType..'\r\n')
	local reset = string.char(0xa5, 0xa5, 0x5a, 0x5a, 0x98, 0xc1, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00)

	-- print (data, #data)

	--[[ MUST
		wifi reset cmd
	]]
	-- print ('start\r\n')
	-- tmpTbl = stringToTable(data)
	-- LOGTBL(tmpTbl)
	-- tmpTbl = stringToTable(reset)
	-- LOGTBL(tmpTbl)
	-- print (string.find(data, reset))
	-- print ('\r\n')
	
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
	return 2
end


-- {"msg":"hello","sDev":{"pid":"0104","did":"0107","clu":"0006","ept":[1, 2],"mac":"7409E17E3376AF60"}}
-- {"ctrl":{"pwr":1},"sDev":{"pid":"0104","did":"0107","clu":"0006","ept":[1, 2],"mac":"7409E17E3376AF60"}}
-- {"status":{"pwr":1,"switcher":1},"sDev":{"pid":"0104","did":"0107","clu":"0006","ept":[1, 2],"mac":"7409E17E3376AF60"}}
-- s1apiGetMacFromIdx()
-- s1apiGetIdxFromMac()


--[[ MUST
]]
-- {"ctrl":{"action":1}}
function s1CvtStd2Pri(json)
	local cvtType = s1apiGetCurrCvtType()
	-- print ('s1CvtStd2Pri return => '..cvtType..'\r\n')
	local tb = cjson.decode(json)
	local ctrl = tb["ctrl"]
	local cmdTbl = { 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfa, 0x44 }
	local dataStr = ""

	-- 打开
	if (ctrl["action"] == 1) then
		cmdTbl[2] = 0x02
	-- 关闭
	elseif (ctrl["action"] == 2) then
		cmdTbl[2] = 0x01
	-- 暂停
	elseif (ctrl["action"] == 3) then
		cmdTbl[2] = 0x03
	-- 测量
	else
		cmdTbl[2] = 0x05
	end
	-- LOGTBL(cmdTbl)

	-- u have to make the bin as string for the return value
	dataStr = tableToString(cmdTbl)
	return string.len(dataStr), dataStr
end

--[[ MUST
	return value: return 0, 0, if the input param 'bin' is not valid
]]
function s1CvtPri2Std(bin)
	local cvtType = s1apiGetCurrCvtType()
	-- print ('s1CvtPri2Std return => '..cvtType..'\r\n')
	local dataTbl = {}
	local str = '{"percentage":%d}'
	dataTbl = stringToTable(bin)
	-- for i = 1, #bin
	-- 		 (bin[i])
	-- end
	-- LOGTBL(dataTbl)

	str = string.format(str, 100 - dataTbl[3])
	-- str = string.format(str, #dataTbl)
	-- print (str)
	return string.len(str), str
end

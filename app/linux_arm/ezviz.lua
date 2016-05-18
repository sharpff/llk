function tableToString( cmd )
	local strcmd = ""
	local i
	
	for i=1, #cmd do
		strcmd = strcmd .. string.char(cmd[i])
	end
	return strcmd
end

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

function s1GetVer()
	local str = '1.0'
	return string.len(str), str
end

function s1GetCvtType()
	local str = string.format('{"whatCvtType":%d,"name":\"%s\"}', 2, "lelink")
	return string.len(str), str, 0
end

--[[ MUST
	查询设备状态。
	每个设备都约定需要一条或者多条指令可以获取到设备的所有状态。
]]
function s1GetQueries()
	local query = string.char( 0xa5, 0xa5, 0x5a, 0x5a, 0xb1, 0xc0, 0x01, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00 )
	local queryCountLen = string.char( 0x0d, 0x00 )

	return string.len( queryCountLen ), queryCountLen, string.len( query ), query
end

--[[ MUST
	WIFI 重置命令判别
]]
function s1GetValidKind(data)
    local reset = string.char( 0xa5, 0xa5, 0x5a, 0x5a, 0x98, 0xc1, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00 )
	if nil ~= string.find(data, reset) then
		-- print '1'
		return 1
	end
	if 14 == #data then
		-- print '2'
		return 2
	end
	return 0
end

function s1CvtStd2Pri(json)
	local tb = cjson.decode(json)
	local ctrl = tb["ctrl"]
	local cmdTbl = { 0xa5, 0xa5, 0x5a, 0x5a, 0x00, 0x00, 0x02, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00 }
	local dataStr = ""
    local sum = 0xbeaf

    for i = 1, 4 do
    	idx = "idx"..i
    	print(idx, ctrl[idx])
    	if(ctrl[idx] ~= nil) then
			cmdTbl[12] = cmdTbl[12] | (0x01 << (i - 1))
    		if(ctrl[idx] == 1) then
				cmdTbl[13] = cmdTbl[13] | (0x01 << (i - 1))
    		end
    	end
    end
    
    for i = 1, #cmdTbl do
        sum = sum + cmdTbl[i]
    end
    cmdTbl[5] = sum & 0xff
    cmdTbl[6] = (sum >> 8) & 0xff

	LOGTBL(cmdTbl)

	-- u have to make the bin as string for the return value
	dataStr = tableToString(cmdTbl)
	return string.len(dataStr), dataStr
end

function s1CvtPri2Std(bin)
    local dataTbl = {}
    local str = '{"idx1":%d,"idx2":%d,"idx3":%d,"idx4":%d}'
    dataTbl = stringToTable(bin)
    -- LOGTBL(dataTbl)

    if dataTbl[1] == 0xa5 and dataTbl[2] == 0xa5 and dataTbl[3] == 0x5a and dataTbl[4] == 0x5a then
        str = string.format(str, (dataTbl[13] >> 0) & 0x1, (dataTbl[13] >> 1) & 0x1, (dataTbl[13] >> 2) & 0x1, (dataTbl[13] >> 3) & 0x1)
    else
        str = ""
    end
    return string.len(str), str
end


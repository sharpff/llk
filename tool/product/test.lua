function tableToString(cmd)
	local strcmd = ""
	local i
	
	for i=1, #cmd do
		strcmd = strcmd .. string.char(cmd[i])
	end
	return strcmd
end

function stringToTable(sta)
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
	local str = string.format('{"whatCvtType":%d,"name":\"%s\"}', 4, "lelink")
	return string.len(str), str, 0
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
function s1GetQueries(queryType)
    local query = ""
    local queryCountLen = string.char(0x00)

    query = string.format('{"DataType":%d}', queryType)

    if queryType == 1 then
    elseif queryType == 2 then
    elseif queryType == 3 then
    elseif queryType == 4 then
    elseif queryType == 5 then
    end
    if string.len(query) ~= 0 then
        queryCountLen = string.char(string.len(query), 0x00)
    end

    return string.len(queryCountLen), queryCountLen, string.len(query), query
end

--[[ MUST
   判断halPipeRead()得到的数据是什么类型的数据
        0, 无效数据
        1, wifi重置数据
        2, 设备状态数据
]]
function s1GetValidKind(bin)
	local tb = cjson.decode(bin) -- {"type":2, "data":"..."}
	return tb["type"]
end

--[[ 
   可以在这里修改halPipeRead()得到的数据,再发向网络
]]
function s1CvtPri2Std(bin)
	local tb = cjson.decode(bin)
    local str = cjson.encode(tb["data"])
    return string.len(str), str
end

--[[ 
   可以在这里修改网络发来的控制数据,传给halPipeWrite()
]]
function s1CvtStd2Pri(json)
	return string.len(json), json
end


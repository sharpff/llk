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
	1. json <-> bin
	2. json <-> json
]]
function getCvtType()
	local whatType = 1
	-- baud - dataBits parity(None:0, Odd:1, Even:2) stopBits
	local baud = '"9600-8N1"'
	local str = string.format('{"whatType":%d,"baud":%s}', whatType, baud)

	return string.len(str), str
end

--[[ MUST
	查询设备状态。

	每个设备都约定需要一条或者多条指令可以获取到设备的所有状态。
	返回的状态由固件来缓存。
	指令必须为完整的指令，可以直接对电控板操作的。
	在用户定义产品时填充。

	标准接口，该方法由云端自动生成。
]]
function getQueries()
	local query = string.char( 0xbb, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfa, 0x44 )
	local queryCountLen = string.char( 0x09, 0x00 )

	return string.len( queryCountLen ), queryCountLen, string.len( query ), query
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
function cvtStd2Pri(json)
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
	LOGTBL(cmdTbl)

	-- u have to make the bin as string for the return value
	dataStr = tableToString(cmdTbl)
	return string.len(dataStr), dataStr
end

--[[ MUST
]]
function cvtPri2Std(bin)
	local dataTbl = {}
	local str = '{"percentage":%d}'
	dataTbl = stringToTable(bin)
	-- for i = 1, #bin
	-- 	print (bin[i])
	-- end
	LOGTBL(dataTbl)

	str = string.format(str, 100 - dataTbl[3])
	-- str = string.format(str, #dataTbl)
	-- print (str)
	return string.len(str), str
end

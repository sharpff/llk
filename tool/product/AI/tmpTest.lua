--[[ 
    THIS IS IA SCRIPT
  ]]

--[[
    INTERNAL
]]
function cmpUTC(utcH, utcL, utcBaseH, utcBaseL) 
    local ret = -1
    if (utcH >= utcBaseH) and (utcL > utcBaseL) then
        ret = 1
    elseif (utcH == utcBaseH) and (utcL == utcBaseL) then
        ret = 0
    end
    return ret
end

--[[
    INTERNAL
]]
function getOldStatus(uuid)
    local l, n = s2GetSelfName()
    local tblOldStatus = s2apiGetLatestStatus(l, n)

    if tblOldStatus then 
        for _, jsonOldStatus in pairs(tblOldStatus) do
            if nil ~= string.find(jsonOldStatus, uuid) then
                local tbl = cjson.decode(jsonOldStatus)
                return tbl["status"]
            end
        end
    end
    return nil
end

--[[
    INTERNAL
]]
function cmpValInOldStatus(uuid, val, token)
    local l, n = s2GetSelfName()
    local ret = -2

    local tblOldStatus = getOldStatus(uuid)
    if tblOldStatus then
        if tblOldStatus[token] < val then
            ret = 1
        elseif tblOldStatus[token] > val then
            ret = -1
        else
            ret = 0
        end
    end
    return ret
end

--[[
    EXPORT
]]
function getBeingReservedInfo()
    -- CUSTOMIZATION START
    local tblInfo = {'10000100091000610006F0B429000012'}
    -- CUSTOMIZATION END
    return tblInfo
end

--[[
    EXPORT
    if the rule has been expired
    selfStatus: json (S2 json format) => {\"status\":{},\"utcH\":339,\"utcL\":136006835}
    E.g. utc:1456129920 (2016/2/22 16:32:00(8+))      utcH: (339)0x153 utcL: 136006835(0x081b4cb3)
    lua5.3.0_win_slim.exe hello/helloworld.lua s2IsValid {\"status\":{},\"utcH\":339,\"utcL\":136006835}
]]
function s2IsValid(selfStatus)
    local valid = 0
    local tb = cjson.decode(selfStatus)
    local utcL = tb["utcL"]
    local utcH = tb["utcH"]
    
    -- E.g. 16/12/1 0:00:00 is 0x583F6800
    -- CUSTOMIZATION START
    local utcBaseH = 0
    local utcBaseL = 1477913349
    -- CUSTOMIZATION END
    if (cmpUTC(utcH, utcL, utcBaseH, utcBaseL) <= 0) then 
        valid = 1
    end
    return 1
end

--[[
    EXPORT
    return value: [repeating call or status trig, AND or OR]
    lua5.3.0_win_slim.exe hello/helloworld.lua s2GetRuleType
]]
function s2GetRuleType()
    local repeatOrTrig = 0
    local isAND = 0
    -- repeating call or status trig, AND or OR
    -- CUSTOMIZATION START
    repeatOrTrig = 0
    isAND = 0 -- ignore it
    -- CUSTOMIZATION END
    return repeatOrTrig, isAND
end

--[[
    EXPORT
    the being reserved node info(uuid(s))
    lua5.3.0_win_slim.exe hello/helloworld.lua s2GetBeingReservedInfo
]]
function s2GetBeingReservedInfo()
    local reserved = ''
    local count = 0
    local tblInfo = getBeingReservedInfo()
    return tblInfo
end

--[[
    ./Debug/lua /home/lf/dev/rp/lelinkDev/tool/product/AI/tmpTest.lua s2IsConditionOKExt 
    "{\"status\":{},\"uuid\":\"10000100091000610006F0B429000012\"}" "{\"status\":{\"percentage\":0},\"uuid\":\"10000100091000610006F0B429000012\"}"
    "{\"status\":{},\"uuid\":\"10000100091000610006F0B429000012\"}" "{\"status\":{\"percentage\":0},\"uuid\":\"10000100091000610006F0B429000012\"}"
]]
function s2IsConditionOKExt(selfStatus, rmtStatus)
    local ok = 0
    local toStoreStatus = 0
    local rmtUUID = nil
    local tblRmt = {}
    if not (nil == rmtStatus) then
        tblRmt = cjson.decode(rmtStatus)
    end
    local tblSelf = {}
    if not (nil == selfStatus) then
        tblSelf = cjson.decode(selfStatus)
    end

    r, a = s2GetRuleType()
    local rmtUUID = tblRmt["uuid"]
    if nil == rmtUUID and 0 == r then
        return ok
    end

    -- CUSTOMIZATION START
    toStoreStatus = 1

    local tblOldStatus = {}
    local tblConditions = {}
    local tblConditionsOld = {}
    local retIdx = 1;
    local tblNewStatus = tblRmt["status"]
    if nil == tblNewStatus then
        return ok
    end

    -- CONDITION(s) START
    tblConditions[retIdx] = 0
    tblConditionsOld[retIdx] = 0
    if nil ~= string.find(rmtUUID, '10000100091000610006F0B429000012') then
        if 50 <= tblNewStatus["percentage"] and 0 ~= cmpValInOldStatus('10000100091000610006F0B429000012', tblNewStatus["percentage"], "percentage") then
            tblConditions[retIdx] = 1
            print('ok 1\r\n')
        end
    else
        local tblOldStatus = getOldStatus('10000100091000610006F0B429000012')
        if tblOldStatus then
            if 50 <= tblOldStatus["percentage"] then
                tblConditionsOld[retIdx] = 1
                print('okOld 1\r\n')
            end
        end
    end
    retIdx = retIdx + 1
    
    
    -- CONDITION(s) END

    if 1 == tblConditions[1] then
        ok = 1
    end

    -- CUSTOMIZATION END

    if ((toStoreStatus == 1) and 
    not (nil == rmtUUID) and 
    not (nil == rmtStatus)) then 
        local l, n = s2GetSelfName()
        s2apiSetCurrStatus(l, n, string.len(rmtUUID), rmtUUID, string.len(rmtStatus), rmtStatus)
    end

    return ok
end


--[[
    EXPORT
    the self rule name (rand rule name, buf diff for every rule names)
    E.g.
    lua5.3.0_win_slim.exe hello/helloworld.lua s2GetSelfName
]]
function s2GetSelfName()
    local name = ''
    -- CUSTOMIZATION START
    name = '2016103119290900021'
    -- CUSTOMIZATION END
    return string.len(name), name
end

--[[
    EXPORT
    the self control cmd(json)
    E.g.
    lua5.3.0_win_slim.exe hello/helloworld.lua s2GetSelfCtrlCmd
]]
function s2GetSelfCtrlCmd()
    --[[
        to get the cmd for self ctrl
    ]]
    local selfCtrl = ''
    
    -- CUSTOMIZATION START
    selfCtrl = '{"speed":1,"pwr":1}'
    -- CUSTOMIZATION END
    return string.len(selfCtrl), selfCtrl
end
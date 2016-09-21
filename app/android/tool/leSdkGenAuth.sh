#!/bin/bash
MAIN_PATH="../../.."
$MAIN_PATH/tool/lelinkTool.py \
    -a iot.fineat.com \
    -p 9011 \
    -u f1b312fd6f7b427f8306 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDDxqkuVDvetJDbq1JgBr93VBYKJZdnZbMFS7gWoNjX1BUTxjBRY+2ek2k/V08lTj9ejlCsnn927lE4iOBzrHY66xIktxsyw6btmGcqQ/doDX6y89mHcbq8e2oFE+eJ/vuqI426g+waaYxPrCtsuN+G+4PUEnZnOZP/PFAGMj3pywIDAQAB \
    -s vUckaTiTQsBuvtKHS1UgNMarBopvFTZolNGXbnkwqD2GX9d7c6dSQOHOr8niTFYPWkhl2/3DQWouRWfMXt55QYKvSpToiQA7EZqcnNR7/T1hfa0RKU8lzuRV3RWD7CVApmTJ5ZIHQMd8GL1kDjj8zPNdgAZKr9Dpnpyz0yfBzaM= \
    -o $MAIN_PATH/app/android/assets/lelink/auth.cfg

    # 测试
    #-a 10.154.252.130 \
    #-p 5546 \

    # 外网, 内网ip
    #-a 10.182.192.19 \
    #-p 9002 \

    # 外网
    #-a 115.182.63.167 \
    #-p 9002 \

    # DNS
    # 测试
    #-a iot.fineat.com \
    #-p 9011 \

    # DNS
    # 外网，待定
    #-a iot.xxx.com \
    #-p 90xx \

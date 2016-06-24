#!/bin/bash

#
# Create on: 2016-06-23
#
#    Author: feiguoyou@hotmail.com
#

echo $(pwd)
PACK_FILES=$PACK_FILES:"airconfig.h"
PACK_FILES=$PACK_FILES:"io.h"
PACK_FILES=$PACK_FILES:"leconfig.h"
PACK_FILES=$PACK_FILES:"ota.h"
PACK_FILES=$PACK_FILES:"protocol.h"
PACK_FILES=$PACK_FILES:"state.h"
PACK_FILES=$PACK_FILES:"mbedtls-2.2.0_crypto/include/mbedtls/*.h@sw/mbedtls-2.2.0_crypto/include/mbedtls"

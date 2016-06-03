#!/bin/bash
MAIN_PATH="../.."
$MAIN_PATH/lelinkTool.py \
    -a 115.182.63.167 \
    -p 9002 \
    -u 10000100171001110017 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCcFM6x4LHwYL86pPztOjDDOcppfwSzS4AAttYvEfo8lfcLP67hcrJCJipPxIwcg0vF0Ravy+GQbigxc8VFwg750DfYCMb0rEllTCe2ShHXL9LAo4JP1JSeryTA0p1UxmZ5nmtwDQ/JKqGOZcv8aafJkrpCn/1Q9RlVJjoDYSK6JQIDAQAB \
    -s S5WIOlB3lXjhkDH4dPnWEcz5lAblDS17UkTW99NojxLhuLasD1DwMISjHvKq/DHQhOzYQ/eaMoUzsaQSkicwP6i0tOzGNCoSagmSnlAXP8weS8J3Ty2XxMYNLHEb/olaPGkwsA1rImK+l8385ZTZsHe3Sn70UPsSQOx8Kz37zyY= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t $MAIN_PATH/product/leyi/story.lua \
    -o ./0x1c3000.bin

$MAIN_PATH/lelinkTool.py \
    --genpriv \
    -o ./0x1c8000.bin


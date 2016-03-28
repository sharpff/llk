#!/bin/bash
# MAIN_PATH="../.."
# $MAIN_PATH/lelinkTool.py \
#     -a 10.154.252.130 \
#     -p 5546 \
#     -u a2077ecbcd0e4684a247 \
#     -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCuVNstUYV2RLwLG7ec0xgv3Mk/vPRZ9ph3NmyYP0flDzPpAVOHSYL9e9kiG1dO3e2QIZxcWcLjcDnYVyzhrW04covZiSwWS9vENHHFh5wOagkOyjSImp8PxgzOYlaP94/J1bI+UphNrQSo+OWYQKSdm4HUvVNcpMvYruXtWkOdmQIDAQAB \
#     -s R03bQyHT5tPsxqwPfFUeALfVaUtqQEX99dwnWFei9O1DWuA+A5j7alPThx5CNMT+aEIcfYIGSQVOJcmc7waQSkNzTcnzmpxWgZo8FXsrsQ3VrC61+FO5VVxZi7g20u1qfxd2Df6ulwZEC9rHC1NYRQd6EZQUoZz3j+1ZRGJRYCc= \
#     -o ./0x1c2000.bin

MAIN_PATH="../.."
$MAIN_PATH/lelinkTool.py \
    -a 10.154.252.130 \
    -p 5546 \
    -u 10000100121000010009 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCHs9E6fH+O2UsKei18yJHcwdUN5UM7siVFWoIgUh8zco8uDPs9LPWfOG380yBBhUaK8NSKu6UtdIA49oJ+Lf9aB5HB38+qz75sqytxdVpZx2I8sv12az7pae5PPzQOrXmUB422j3iuojzPRaioM6OlNVrZFCmoFbccCJbcovf3mQIDAQAB \
    -s INI+GRhvppjAhZ+AGcK2yMoz97Espx1L8NEqZW6dZsv2Ykz7kaiLxQsHSoNcWlc1PLiaMa0K+YdU5CHJJw63X0a6WOY8EH2qFPATmjl+4xQvjJGwxPnYYlGgQh3+z4CIkb9UoHkxvy4/uc2QEImIOPGZlUEo5685Ko5l9YGmK2U= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t ./qca9531.lua \
    -o ./0x1c3000.bin

# $MAIN_PATH/lelinkTool.py \
#     --genpriv \
#     -o ./0x1c8000.bin

# $MAIN_PATH/lelinkTool.py \
#     -t $MAIN_PATH/product/AI/genIt/helloworld.lua \
#     -o ./0x1c9000.bin

# $MAIN_PATH/lelinkTool.py \
#     ./0x1c2000.bin 0x1000 \
#     ./0x1c3000.bin 0x5000 \
#     -o ./cust.bin

# $MAIN_PATH/lelinkTool.py \
#     $MAIN_PATH/boardMW300/boot2.bin 0x4000 \
#     $MAIN_PATH/boardMW300/layout.bin 0x2000 \
#     $MAIN_PATH/boardMW300/psm.bin 0x4000 \
#     $MAIN_PATH/le_demo.bin 0x154000 \
#     $MAIN_PATH/boardMW300/300_WIFI.bin 0x64000 \
#     ./cust.bin 0x3E000 \
#     -o ./image.bin


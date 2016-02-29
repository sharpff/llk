300_HEAD.bin:
boot2.bin  8k
layout.bin 4k   通过Marvell的flashprog.sh，先生成到board上面，再读取出来
PSM.bin    8k
config.bin 4k

boot2.bin使用的是3.2.16的，否则会导致GD的flash16Mbit不能片上执行。





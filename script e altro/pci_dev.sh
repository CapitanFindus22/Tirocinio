#!/bin/sh

mknod /dev/disp c 247 0
insmod /lib/modules/prova/dev_mod.ko

#!/bin/sh

mknod /dev/disp c 247 0
insmod /lib/modules/dev_mod.ko

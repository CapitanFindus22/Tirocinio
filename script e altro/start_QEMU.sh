#!/bin/bash
qemu-system-x86_64 -s -S -m 256M -kernel linux-6.14.7/arch/x86_64/boot/bzImage -initrd rootfs.cpio.gz -append "root=/dev/mem" &
sleep 1
seergdb ./linux-6.14.7/vmlinux --silent 
#Inserire target remote :1234 in gdb commands before "run"

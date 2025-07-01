#!/bin/bash

KERNEL_PATH=linux-6.14.7/arch/x86/boot/bzImage
VMLINUX=linux-6.14.7/vmlinux
INITRD=rootfs.cpio.gz

MACHINE_OPTS="-m 256M -kernel $KERNEL_PATH -initrd $INITRD -append 'root=/dev/mem' -machine q35 -device disp,bus=pcie.0"

if [[ "$1" == "gdb" ]]; then
    echo "[*] Avvio in modalità debug..."
    qemu-system-x86_64 -s -S $MACHINE_OPTS -monitor stdio &
    sleep 1
    seergdb $VMLINUX --silent target remote:1234
else
    echo "[*] Avvio normale..."
    qemu-system-x86_64 $MACHINE_OPTS
fi

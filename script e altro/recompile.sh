#!/bin/bash

gcc a.c -static -O2 -o a -lm
gcc print_reg.c -static -O2 -o ../rootfs/reg

cd ../library || exit 1
gcc test.c -static -O2 -o ../rootfs/test -lm
gcc test.c -static -O2 -DNO_DEV -o ../rootfs/testb -lm
gcc test2.c -static -O2 -o ../rootfs/test2 -lm
gcc test2.c -static -O2 -DNO_DEV -o ../rootfs/test2b -lm
gcc test3.c -static -O2 -o ../rootfs/test3
gcc test3.c -static -O2 -DNO_DEV -o ../rootfs/test3b -lm
cd ../modulo || exit 1
make || exit 1
cd ../rootfs || exit 1
find . -print0 | cpio --null -ov --format=newc | gzip -9 >../rootfs.cpio.gz

cd ..
cp disp/disp.c qemu-10.0.2/hw/pci || exit 1
cd qemu-10.0.2 || exit 1
make -j"$(nproc)" || exit 1
sudo make install || exit 1


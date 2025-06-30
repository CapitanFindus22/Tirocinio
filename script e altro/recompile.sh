#!/bin/bash

cd rootfs/lib/modules/prova/
make
cd ../../..
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../rootfs.cpio.gz
cd ~/Scaricati/qemu-10.0.2
sudo make -j$(nproc)
sudo make install

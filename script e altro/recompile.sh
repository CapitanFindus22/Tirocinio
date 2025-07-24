#!/bin/bash

build_fs() {
  cd ~/Scrivania/Tesi/Emulatore/library
  gcc test.c -static -o ../rootfs/test -lm
  gcc test2.c -static -o ../rootfs/test2 -lm
  gcc test3.c -static -o ../rootfs/test3
  cd ~/Scrivania/Tesi/Emulatore/modulo || exit 1
  make || exit 1
  cd ../rootfs || exit 1
  find . -print0 | cpio --null -ov --format=newc | gzip -9 >../rootfs.cpio.gz
}

build_qemu() {
  cp disp/disp.c ~/Scaricati/qemu-10.0.2/hw/pci
  cd ~/Scaricati/qemu-10.0.2 || exit 1
  #./configure --target-list=x86_64-softmmu || exit 1
  make -j"$(nproc)" || exit 1
  sudo make install || exit 1
}

case "$1" in
all)
  build_qemu
  build_fs
  ;;
fs)
  build_fs
  ;;
qemu)
  build_qemu
  ;;
*)
  exit 1
  ;;
esac

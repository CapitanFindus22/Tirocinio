#!/bin/bash

build_fs() {
  cd rootfs/lib/modules/prova/ || exit 1
  make || exit 1
  cd ../../.. || exit 1
  find . -print0 | cpio --null -ov --format=newc | gzip -9 >../rootfs.cpio.gz
}

build_qemu() {
  cd ~/Scaricati/qemu-10.0.2 || exit 1
  sudo make -j"$(nproc)" || exit 1
  sudo make install || exit 1
}

case "$1" in
all)
  build_fs
  build_qemu
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

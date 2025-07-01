#!/bin/sh

if [ $# -ne 1 ]; then
  echo "Usage: $0 <PCI(e)_config_space_file>"
  exit 1
fi

CONFIG=$1

# Legge n byte da offset e ritorna stringa hex, invertita se più di 1 byte (LE -> BE)
read_le() {
  raw=$(xxd -p -s "$1" -l "$2" "$CONFIG" | tr -d '\n')
  if [ "$2" -le 1 ]; then
    echo "$raw"
  else
    # Inverti byte per little endian
    len=${#raw}
    res=""
    i=$((len - 2))
    while [ $i -ge 0 ]; do
      res="${res}${raw:$i:2}"
      i=$((i - 2))
    done
    echo "$res"
  fi
}

# Legge 4 byte da offset come uint32 LE (decimale)
read_le_uint32() {
  raw=$(xxd -p -s "$1" -l 4 "$CONFIG" | tr -d '\n')
  b0=${raw:0:2}
  b1=${raw:2:2}
  b2=${raw:4:2}
  b3=${raw:6:2}
  echo $((0x$b3$b2$b1$b0))
}

filesize=$(stat -c %s "$CONFIG")

echo "Vendor ID: 0x$(read_le 0 2)"
echo "Device ID: 0x$(read_le 2 2)"
echo "Command: 0x$(read_le 4 2)"
echo "Status: 0x$(read_le 6 2)"
echo "Revision ID: 0x$(read_le 8 1)"
echo "Class Code: 0x$(read_le 9 3)"
echo "Header Type: 0x$(read_le 14 1)"
echo "Capability Pointer: 0x$(read_le 52 1)"

echo
echo "=== Capabilities ==="

cap_ptr=$((0x$(read_le 52 1)))

while [ "$cap_ptr" -ne 0 ]; do
  cap_id=$(read_le "$cap_ptr" 1)
  next_ptr=$(read_le $((cap_ptr + 1)) 1)

  printf "Capability at offset 0x%02x: ID=0x%s, Next=0x%s\n" "$cap_ptr" "$cap_id" "$next_ptr"

  cap_ptr=$((0x$next_ptr))
done

if [ "$filesize" -le 256 ]; then
  echo
  echo "Extended capability space not present (config space size <= 256 bytes)."
  exit 0
fi

echo
echo "=== Extended Capabilities ==="

ext_cap_ptr=256

while [ "$ext_cap_ptr" -ne 0 ]; do
  val=$(read_le_uint32 "$ext_cap_ptr")

  cap_id=$(( val & 0xFFFF ))
  version=$(( (val >> 16) & 0xF ))
  next_ptr=$(( (val >> 20) & 0xFFF ))

  printf "Extended Capability at offset 0x%03x: ID=0x%04x, Version=0x%x, Next=0x%03x\n" \
    "$ext_cap_ptr" "$cap_id" "$version" "$next_ptr"

  [ "$next_ptr" -eq 0 ] && break
  [ "$next_ptr" -eq "$ext_cap_ptr" ] && break

  ext_cap_ptr=$next_ptr
done

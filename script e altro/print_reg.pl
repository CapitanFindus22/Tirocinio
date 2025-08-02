open F, "<", "/sys/bus/pci/devices/0000:01:00.0/config";
seek F, 0x10, SEEK_SET;
read F, $d, 4;
printf "%08x\n", unpack("V", $d);


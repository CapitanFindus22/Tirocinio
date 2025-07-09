#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

bool express = false;

uint8_t read_u8(int fd, off_t offset) {
    uint8_t val;
    if (pread(fd, &val, 1, offset) != 1) {
        perror("read_u8");
        exit(1);
    }
    return val;
}

uint16_t read_u16(int fd, off_t offset) {
    uint8_t b[2];
    if (pread(fd, b, 2, offset) != 2) {
        perror("read_u16");
        exit(1);
    }
    return b[0] | (b[1] << 8);
}

uint32_t read_u32(int fd, off_t offset) {
    uint8_t b[4];
    if (pread(fd, b, 4, offset) != 4) {
        perror("read_u32");
        exit(1);
    }
    return b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
}

const char* pci_capability_name(uint8_t id) {
    switch (id) {
        case 0x00: return "Null Capability";
        case 0x01: return "Power Management";
        case 0x02: return "AGP";
        case 0x03: return "Vital Product Data (VPD)";
        case 0x04: return "Slot Identification";
        case 0x05: return "MSI";
        case 0x06: return "CompactPCI Hot Swap";
        case 0x07: return "PCI-X Capabilities";
        case 0x08: return "HyperTransport";
        case 0x09: return "Vendor Specific";
        case 0x0A: return "Debug Port";
        case 0x0B: return "CompactPCI Central Resource Control";
        case 0x0C: return "PCI Hot-Plug";
        case 0x0D: return "Bridge Subsystem Vendor ID";
        case 0x0E: return "AGP 8x";
        case 0x0F: return "Secure Device";
        case 0x10: 
            express = true;
            return "PCI Express";
        case 0x11: return "MSI-X";
        case 0x12: return "SATA Data/Index Configuration";
        case 0x13: return "Advanced Features (AF)";
        case 0x14: return "Enhanced Allocation";
        case 0x15: return "Flattening Portal Bridge";
        default: return "Unknown Capability";
    }
}

const char* pci_ext_capability_name(uint16_t id) {
    switch (id) {
        case 0x0000: return "Null Capability";
        case 0x0001: return "Advanced Error Reporting (AER)";
        case 0x0002: return "Virtual Channel (VC)";
        case 0x0003: return "Device Serial Number (DSN)";
        case 0x0004: return "Power Budgeting";
        case 0x0005: return "Root Complex Link Declaration";
        case 0x0006: return "Root Complex Internal Link Control";
        case 0x0007: return "Root Complex Event Collector (RCEC)";
        case 0x0008: return "Multi-Function Virtual Channel (MFVC)";
        case 0x0009: return "Virtual Channel (with MFVC)";
        case 0x000A: return "Root Complex Register Block (RCRB)";
        case 0x000B: return "Vendor-Specific Extended Capability";
        case 0x000C: return "Config Access Correlation (CAC)";
        case 0x000D: return "Access Control Services (ACS)";
        case 0x000E: return "Alternate Routing-ID Interpretation (ARI)";
        case 0x000F: return "Address Translation Services (ATS)";
        case 0x0010: return "Single Root I/O Virtualization (SR-IOV)";
        case 0x0011: return "Multi Root I/O Virtualization (MR-IOV)";
        case 0x0012: return "Multicast (MCAST)";
        case 0x0013: return "Page Request Interface (PRI)";
        case 0x0015: return "Resizable BAR (ReBAR)";
        case 0x0016: return "Dynamic Power Allocation (DPA)";
        case 0x0017: return "TPH Requester";
        case 0x0018: return "Latency Tolerance Reporting (LTR)";
        case 0x0019: return "Secondary PCIe Capability";
        case 0x001A: return "Protocol Multiplexing (PMUX)";
        case 0x001B: return "Process Address Space ID (PASID)";
        case 0x001D: return "Downstream Port Containment (DPC)";
        case 0x001E: return "L1 PM Substates (L1SS)";
        case 0x001F: return "Precision Time Measurement (PTM)";
        case 0x0023: return "Designated Vendor-Specific Extended Capability (DVSEC)";
        case 0x0025: return "Data Link Feature (DLF)";
        case 0x0026: return "Physical Layer 16.0 GT/s";
        case 0x0027: return "Lane Margining at the Receiver (LMR)";
        case 0x0028: return "Hierarchy ID (HID)";
        case 0x0029: return "Native PCIe Enclosure Management (NPEM)";
        case 0x002A: return "Physical Layer 32.0 GT/s";
        case 0x002B: return "Alternate Protocol Capability";
        case 0x002C: return "System Firmware Intermediary (SFI)";
        default:     return "Unknown Extended Capability";
    }
}

int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "Inserire un indirizzo\n", argv[0]);
        return -1;
    }

    char path[256];

    snprintf(path,sizeof(path),"/sys/bus/pci/devices/%s/config",argv[1]);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Errore apertura file config PCI");
        return 1;
    }

    off_t cap_ptr = read_u8(fd, 0x34);
    uint8_t head_type = read_u8(fd, 0x0e);

    printf("[PCI Header]\n");
    printf("Vendor ID:      0x%04x\n", read_u16(fd, 0x00));
    printf("Device ID:      0x%04x\n", read_u16(fd, 0x02));
    printf("Command:        0x%04x\n", read_u16(fd, 0x04));
    printf("Status:         0x%04x\n", read_u16(fd, 0x06));
    printf("HeaderType:     0x%02x\n", head_type);
    printf("Cap. pointer:   0x%02x\n\n", cap_ptr);

    printf("BAR0:           0x%04x\n", read_u32(fd, 0x10));
    printf("BAR1:           0x%04x\n", read_u32(fd, 0x14));

    if (!(head_type & 1))
    {
        printf("BAR2:           0x%04x\n", read_u32(fd, 0x18));
        printf("BAR3:           0x%04x\n", read_u32(fd, 0x20));
        printf("BAR4:           0x%04x\n", read_u32(fd, 0x24));
        printf("BAR5:           0x%04x\n", read_u32(fd, 0x28));
    }

    printf("\n");

    if (cap_ptr != 0)
    {

        uint8_t id;

        while (cap_ptr != 0)
        {

            id = read_u8(fd, cap_ptr);
            printf("Capability:     %s\n", pci_capability_name(id));
            cap_ptr = read_u8(fd, cap_ptr + 1);
        }

        printf("\n");

        if (express)
        {
            
            cap_ptr = 0x100;
            uint16_t id;
            uint8_t version;

            while (cap_ptr != 0)
            {

                id = read_u16(fd, cap_ptr);
                cap_ptr = read_u16(fd, cap_ptr + 2);
                version = cap_ptr & 0xf;
                printf("Ext. Capability:  %s, version %01x\n", pci_ext_capability_name(id), version);
                cap_ptr /= 0x10;
            }
        }
        
    }
        

    return 0;
}
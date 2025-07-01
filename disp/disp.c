#include "qemu/osdep.h"
#include "qemu/units.h"
#include "hw/pci/pci.h"
#include "hw/pci/pcie.h"
#include "hw/hw.h"
#include "hw/pci/msi.h"
#include "qemu/timer.h"
#include "qom/object.h"
#include "qemu/main-loop.h"
#include "qemu/module.h"
#include "qapi/visitor.h"
#include "exec/memory.h"
#include <signal.h>

#define TYPE_PCI_CUSTOM_DEVICE "disp"

// Struct defining/describing the state
// of the custom pci device.
typedef struct PciDevState
{
    PCIDevice pdev;
    MemoryRegion mmio_bar0;
    MemoryRegion mmio_bar1;
    uint32_t bar0[4];
    uint8_t bar1[4096];
    dma_addr_t addr;
    dma_addr_t size;
    AddressSpace *dma_as;
    void *tmp_pointer;

} PciDevState;

DECLARE_INSTANCE_CHECKER(PciDevState, PCIDEV, TYPE_PCI_CUSTOM_DEVICE)

/*****************************
 *       BAR0 operations     *
 *****************************/
static uint64_t pcidev_bar0_mmio_read(void *opaque, hwaddr addr, unsigned size)
{
    PciDevState *pcidev = opaque;
    printf("PCIDEV: BAR0 pcidev_mmio_read() addr %lx size %x \n", addr, size);

    return pcidev->bar0[addr / 4];
}

static void pcidev_bar0_mmio_write(void *opaque, hwaddr addr, uint64_t val,
                                   unsigned size)
{
    printf("PCIDEV: BAR0 pcidev_mmio_write() addr %lx size %x val %lx \n", addr, size, val);
    PciDevState *pcidev = opaque;

    if (addr >= 64)
        return;

    pcidev->bar0[addr / 4] = val;
}

static const MemoryRegionOps pcidev_bar0_mmio_ops = {
    .read = pcidev_bar0_mmio_read,
    .write = pcidev_bar0_mmio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },

};

/*****************************
 *       BAR1 operations     *
 *****************************/
static uint64_t pcidev_bar1_mmio_read(void *opaque, hwaddr addr, unsigned size)
{

    PciDevState *pcidev = opaque;
    printf("PCIDEV: BAR1 pcidev_mmio_read() addr %lx size %x \n", addr, size);

    uint16_t val = 0;

    ((uint8_t *)pcidev->tmp_pointer)[0] = 2;

    for (size_t i = 0; i < pcidev->size; i++)
    {
        val += ((uint8_t *)pcidev->tmp_pointer)[i];
    }

    printf("VALUE: %u\n", val);

    munlock(pcidev->tmp_pointer, pcidev->size);
    cpu_physical_memory_unmap(pcidev->tmp_pointer, pcidev->size, true, pcidev->size);

    return val;
}

static void pcidev_bar1_mmio_write(void *opaque, hwaddr addr, uint64_t val,
                                   unsigned size)
{

    printf("PCIDEV: BAR1 pcidev_mmio_write() addr %lx size %x val %lx \n", addr, size, val);
    PciDevState *pcidev = opaque;

    if (size == 8)
    {
        pcidev->addr = val;
        return;
    }

    if (size == 1)
    {
        pcidev->size = (uint8_t)val;
        pcidev->tmp_pointer = cpu_physical_memory_map(pcidev->addr, &pcidev->size, true);
        mlock(pcidev->tmp_pointer, pcidev->size);
        return;
    }
}

static const MemoryRegionOps pcidev_bar1_mmio_ops = {
    .read = pcidev_bar1_mmio_read,
    .write = pcidev_bar1_mmio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 1,
        .max_access_size = 8,
    },
    .impl = {
        .min_access_size = 1,
        .max_access_size = 8,
    },
};

static void pci_print_capabilities(PCIDevice *pdev)
{
    uint8_t cap_ptr = pdev->config[PCI_CAPABILITY_LIST];
    uint16_t vendor_id = pci_get_word(pdev->config + PCI_VENDOR_ID);
    uint16_t device_id = pci_get_word(pdev->config + PCI_DEVICE_ID);

    printf("PCI capabilities list for device %04x:%04x at bus %d slot %d func %d:\n",
           vendor_id, device_id,
           pci_dev_bus_num(pdev), PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn));

    while (cap_ptr != 0)
    {
        uint8_t cap_id = pdev->config[cap_ptr];
        uint8_t next_ptr = pdev->config[cap_ptr + 1];

        printf("  Capability ID 0x%02x at offset 0x%02x, next at 0x%02x\n",
               cap_id, cap_ptr, next_ptr);

        cap_ptr = next_ptr;
    }
}

static void pci_print_ext_capabilities(PCIDevice *pdev)
{
    uint16_t cap_ptr = 0x100; // Extended capabilities start here

    if (cap_ptr >= PCIE_CONFIG_SPACE_SIZE)
    {
        printf("No PCI Express Extended Capabilities found (invalid start offset).\n");
        return;
    }

    int count = 0;
    printf("PCI Express Extended Capabilities list:\n");

    while (cap_ptr != 0)
    {
        if (cap_ptr < 0x100 || cap_ptr > PCIE_CONFIG_SPACE_SIZE - 4)
        {
            printf("Invalid extended capability pointer: 0x%x\n", cap_ptr);
            break;
        }

        uint32_t cap_header = pci_get_long(pdev->config + cap_ptr);

        uint16_t cap_id = cap_header & 0xFFFF;
        uint8_t version = (cap_header >> 16) & 0xF;
        uint16_t next_ptr = (cap_header >> 20) & 0xFFF;

        if (cap_id == 0)
        {
            // No more capabilities
            break;
        }

        printf("  Extended Capability ID 0x%04x (v%d) at offset 0x%x, next 0x%x\n",
               cap_id, version, cap_ptr, next_ptr);

        count++;
        if (count > 20)
        {
            printf("Too many extended capabilities, possible loop.\n");
            break;
        }

        if (next_ptr == 0 || next_ptr <= cap_ptr)
        {
            // End of list or invalid pointer
            break;
        }

        cap_ptr = next_ptr;
    }

    if (count == 0)
    {
        printf("No PCI Express Extended Capabilities found.\n");
    }
}

// implementation of the realize function.
static void pci_pcidev_realize(PCIDevice *pdev, Error **errp)
{
    PciDevState *pcidev = PCIDEV(pdev);

    /// initial configuration of registers
    memset(pcidev->bar0, 0, 16);
    memset(pcidev->bar1, 0, 4096);

    // Initialize an I/O memory region(pcidev->mmio).
    // Accesses to this region will cause the callbacks
    // of the pcidev_mmio_ops to be called.
    memory_region_init_io(&pcidev->mmio_bar0, OBJECT(pcidev), &pcidev_bar0_mmio_ops, pcidev, "pcidev-mmio-0", 16);
    pci_register_bar(pdev, 0, PCI_BASE_ADDRESS_SPACE_MEMORY, &pcidev->mmio_bar0);

    memory_region_init_io(&pcidev->mmio_bar1, OBJECT(pcidev), &pcidev_bar1_mmio_ops, pcidev, "pcidev-mmio-1", 4096);
    pci_register_bar(pdev, 1, PCI_BASE_ADDRESS_SPACE_MEMORY, &pcidev->mmio_bar1);

    pcidev->dma_as = pci_get_address_space(pdev);

    pci_config_set_interrupt_pin(pdev->config, 1);

    // Add capabilities

    // Express

    cap_offset = pcie_endpoint_cap_init(pdev, 0);
    pci_set_word(pdev->config + cap_offset + PCI_EXP_FLAGS, (PCI_EXP_TYPE_ENDPOINT << 4));

    // Power management
    uint8_t cap_offset = pci_add_capability(pdev, PCI_CAP_ID_PM, 0x00, 8, errp);
    pci_set_byte(pdev->config + cap_offset + 2, 0x00);
    pci_set_word(pdev->config + cap_offset + 4, 0x0000);

    // MSI
    msi_init(pdev, 0, 1, true, false, errp);

    // Extended

    int offset = 0x100;

    // Serial number
    pcie_add_capability(pdev, 0x3, 1, offset, 12);
    pci_set_long(pdev->config + offset + 4, 0x12345678);
    pci_set_long(pdev->config + offset + 8, 0x9ABCDEF0);

    offset += 12;

    pci_print_capabilities(pdev);

    pci_print_ext_capabilities(pdev);
}

static void pci_pcidev_uninit(PCIDevice *pdev)
{
    return;
}

static void pcidev_instance_init(Object *obj)
{
    return;
}

static void pcidev_class_init(ObjectClass *class, void *data)
{
    PCIDeviceClass *k = PCI_DEVICE_CLASS(class);

    k->realize = pci_pcidev_realize;
    k->exit = pci_pcidev_uninit;
    k->vendor_id = PCI_VENDOR_ID_QEMU;
    k->device_id = 0xbeef;
    k->revision = 0x10;
    k->class_id = PCI_CLASS_OTHERS;
    k->subsystem_id = 0x654;
    k->subsystem_vendor_id = 0xafc;
}

static void pci_custom_device_register_types(void)
{
    static InterfaceInfo interfaces[] = {
        {INTERFACE_PCIE_DEVICE},
        {},
    };
    static const TypeInfo custom_pci_device_info = {
        .name = TYPE_PCI_CUSTOM_DEVICE,
        .parent = TYPE_PCI_DEVICE,
        .instance_size = sizeof(PciDevState),
        .instance_init = pcidev_instance_init,
        .class_init = pcidev_class_init,
        .interfaces = interfaces,
    };

    type_register_static(&custom_pci_device_info);
}

type_init(pci_custom_device_register_types)
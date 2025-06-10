#include "qemu/osdep.h"
#include "qemu/units.h"
#include "hw/pci/pci.h"
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

// Struct defining/descring the state
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
}

static void pci_custom_device_register_types(void)
{
    static InterfaceInfo interfaces[] = {
        {INTERFACE_CONVENTIONAL_PCI_DEVICE},
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
#include "qemu/osdep.h"
#include "hw/pci/pci.h"
#include "hw/pci/pcie.h"
#include "hw/pci/pcie_regs.h"
#include "hw/pci/msi.h"
#include "exec/memory.h"
#include "qom/object.h"
#include "qemu/module.h"
#include </home/leonardo/Scrivania/Tesi/Emulatore/library/cmd.h>

#define TYPE_PCI_CUSTOM_DEVICE "disp"

// Struct defining/describing the state
// of the custom pci device.
typedef struct PciDevState
{
    PCIDevice pdev;
    MemoryRegion mmio_bar0;
    MemoryRegion mmio_bar1;
    MemoryRegion mmio_bar2;
    uint8_t bar0[4];
    hwaddr bar1;
    uint32_t bar2[64];

    AddressSpace *as;
    void *addr;
    hwaddr size;

} PciDevState;

DECLARE_INSTANCE_CHECKER(PciDevState, PCIDEV, TYPE_PCI_CUSTOM_DEVICE)

// Prototypes
void clean(PciDevState*);
void add1(PciDevState*);
void togrey(PciDevState*);

void clean(PciDevState *pcidev) {

    for (short i = 0; i < 64; i++)
    {
        pcidev->bar2[i] = 0;
    }

    *((uint32_t*)pcidev->bar0) = 0;

}

void add1(PciDevState *pcidev)
{

    mlock(pcidev->addr, pcidev->bar2[0] * pcidev->bar2[1] * sizeof(int));

    for (size_t i = 0; i < pcidev->bar2[0]; i++)
    {
        for (size_t j = 0; j < pcidev->bar2[1]; j++)
        {
            printf("%d ", ((int *)pcidev->addr)[i * pcidev->bar2[1] + j]);
            ((int *)pcidev->addr)[i * pcidev->bar2[1] + j]++;
        }

        printf("\n");
    }

    munlock(pcidev->addr, pcidev->bar2[0] * pcidev->bar2[1] * sizeof(int));

    clean(pcidev);

    msi_notify(&pcidev->pdev, 0);

    return;
}

void togrey(PciDevState *pcidev)
{

    short r,g,b;
    short grey;

    mlock(pcidev->addr, pcidev->bar2[0] * pcidev->bar2[1] * sizeof(RGB));

    for (size_t i = 0; i < pcidev->bar2[0]; i++)
    {
        for (size_t j = 0; j < pcidev->bar2[1]; j++)
        {
            r = ((RGB *)pcidev->addr)[i * pcidev->bar2[1] + j].r;
            g = ((RGB *)pcidev->addr)[i * pcidev->bar2[1] + j].g;
            b = ((RGB *)pcidev->addr)[i * pcidev->bar2[1] + j].b;

            printf("\x1b[48;2;%d;%d;%dm  \x1b[0m", r, g, b);

            grey = 0.299*r+0.587*g+0.114*b;

            ((RGB *)pcidev->addr)[i * pcidev->bar2[1] + j].r = grey;
            ((RGB *)pcidev->addr)[i * pcidev->bar2[1] + j].g = grey;
            ((RGB *)pcidev->addr)[i * pcidev->bar2[1] + j].b = grey;

        }

        printf("\n");

    }

    munlock(pcidev->addr, pcidev->bar2[0] * pcidev->bar2[1] * sizeof(RGB));

    clean(pcidev);

    msi_notify(&pcidev->pdev, 0);

    return;
}

/*****************************
 *       BAR0 operations     *
 *****************************/
static uint64_t pcidev_bar0_mmio_read(void *opaque, hwaddr addr, unsigned size)
{
    PciDevState *pcidev = opaque;
    uint64_t val = 0;

    // Controllo bounds
    if (addr + size > sizeof(pcidev->bar0))
    {
        printf("BAR0 read out of bounds\n");
        return 0;
    }

    val = *(uint32_t *)(pcidev->bar0 + addr);

    printf("PCIDEV: BAR0 read addr %lx size %x val %lx\n", addr, size, val);
    return val;
}

static void pcidev_bar0_mmio_write(void *opaque, hwaddr addr, uint64_t val, unsigned size)
{
    PciDevState *pcidev = opaque;

    if (addr + size > sizeof(pcidev->bar0))
    {
        printf("BAR0 write out of bounds\n");
        return;
    }

    switch ((uint32_t)val)
    {
    case add_1:
        add1(pcidev);
        break;
    case to_grey:
        togrey(pcidev);
        break;

    default:
        break;
    }

    printf("PCIDEV: BAR0 write addr %lx size %x val %lx\n", addr, size, val);
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
    if (addr != 0 || size != sizeof(dma_addr_t))
    {
        printf("BAR1 read invalid addr/size\n");
        return 0;
    }
    printf("PCIDEV: BAR1 read pointer %lx\n", (unsigned long)pcidev->bar1);
    return pcidev->bar1;
}

static void pcidev_bar1_mmio_write(void *opaque, hwaddr addr, uint64_t val, unsigned size)
{
    PciDevState *pcidev = opaque;
    if (addr != 0 || size != sizeof(dma_addr_t))
    {
        printf("BAR1 write invalid addr/size\n");
        return;
    }
    pcidev->bar1 = val;

    pcidev->size = 4096;

    pcidev->addr = cpu_physical_memory_map(pcidev->bar1, &pcidev->size, true);

    printf("PCIDEV: BAR1 write pointer %lx\n", (unsigned long)val);
}

static const MemoryRegionOps pcidev_bar1_mmio_ops = {
    .read = pcidev_bar1_mmio_read,
    .write = pcidev_bar1_mmio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = sizeof(dma_addr_t),
        .max_access_size = sizeof(dma_addr_t),
    },
    .impl = {
        .min_access_size = sizeof(dma_addr_t),
        .max_access_size = sizeof(dma_addr_t),
    },
};

/*****************************
 *       BAR2 operations     *
 *****************************/
static uint64_t pcidev_bar2_mmio_read(void *opaque, hwaddr addr, unsigned size)
{
    PciDevState *pcidev = opaque;

    if (size != 4 || addr % 4 != 0 || addr >= sizeof(pcidev->bar2))
    {
        printf("BAR2 read out of bounds or unaligned (addr=%lx size=%u)\n", addr, size);
        return 0;
    }

    uint32_t val = pcidev->bar2[addr / 4];

    printf("PCIDEV: BAR2 read addr 0x%lx size %u val 0x%x\n", addr, size, val);
    return val;
}

static void pcidev_bar2_mmio_write(void *opaque, hwaddr addr, uint64_t val, unsigned size)
{
    PciDevState *pcidev = opaque;

    if (size != 4 || addr % 4 != 0 || addr >= sizeof(pcidev->bar2))
    {
        printf("BAR2 write out of bounds or unaligned (addr=%lx size=%u)\n", addr, size);
        return;
    }

    pcidev->bar2[addr / 4] = val;

    printf("PCIDEV: BAR2 write addr 0x%lx size %u val 0x%lx\n", addr, size, val);
}

static const MemoryRegionOps pcidev_bar2_mmio_ops = {
    .read = pcidev_bar2_mmio_read,
    .write = pcidev_bar2_mmio_write,
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

// implementation of the realize function.
static void pci_pcidev_realize(PCIDevice *pdev, Error **errp)
{
    PciDevState *pcidev = PCIDEV(pdev);

    memset(pcidev->bar0, 0, sizeof(pcidev->bar0));
    memset(pcidev->bar2, 0, sizeof(pcidev->bar2));
    pcidev->bar1 = 0;

    memory_region_init_io(&pcidev->mmio_bar0, OBJECT(pcidev), &pcidev_bar0_mmio_ops, pcidev, "pcidev-mmio-0", sizeof(pcidev->bar0));
    pci_register_bar(pdev, 0, PCI_BASE_ADDRESS_SPACE_MEMORY, &pcidev->mmio_bar0);

    memory_region_init_io(&pcidev->mmio_bar1, OBJECT(pcidev), &pcidev_bar1_mmio_ops, pcidev, "pcidev-mmio-1", sizeof(dma_addr_t));
    pci_register_bar(pdev, 1, PCI_BASE_ADDRESS_SPACE_MEMORY, &pcidev->mmio_bar1);

    memory_region_init_io(&pcidev->mmio_bar2, OBJECT(pcidev), &pcidev_bar2_mmio_ops, pcidev, "pcidev-mmio-2", sizeof(pcidev->bar2));
    pci_register_bar(pdev, 2, PCI_BASE_ADDRESS_SPACE_MEMORY, &pcidev->mmio_bar2);

    pci_config_set_interrupt_pin(pdev->config, 1);

    pcidev->as = pci_get_address_space(PCI_DEVICE(&pcidev->pdev));

    // Add capabilities

    // Express
    uint8_t cap_offset = pcie_endpoint_cap_init(pdev, 0);
    pci_set_word(pdev->config + cap_offset + PCI_EXP_FLAGS, (PCI_EXP_TYPE_ENDPOINT << 4));

    // Power management
    pci_pm_init(pdev, 0, errp);

    // MSI
    msi_init(pdev, 0, 1, true, false, errp);

    // Extended

    int offset = 0x100;

    // Serial number
    pcie_add_capability(pdev, 0x3, 1, offset, 12);
    pci_set_long(pdev->config + offset + 4, 0x12345678);
    pci_set_long(pdev->config + offset + 8, 0x9ABCDEF0);

    offset += 12;
}

static void pci_pcidev_uninit(PCIDevice *pdev)
{
    PciDevState *pcidev = PCIDEV(pdev);

    cpu_physical_memory_unmap(pcidev->addr, pcidev->size, true, pcidev->size);

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
#include "qemu/osdep.h"
#include "hw/pci/pci.h"
#include "hw/pci/pcie.h"
#include "hw/pci/pcie_regs.h"
#include "hw/pci/msi.h"
#include "exec/memory.h"
#include "qom/object.h"
#include "qemu/module.h"
#include </home/leonardo/Scrivania/Tesi/Emulatore/library/cmd.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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

void clean(PciDevState *);
void add1(PciDevState *);
void togrey(PciDevState *);
void convol(PciDevState *);

void clean(PciDevState *pcidev)
{

    memset(pcidev->bar2, 0, sizeof(pcidev->bar2));

    memset(pcidev->bar0, 0, sizeof(pcidev->bar0));
}

/**
 * Add 1 to every element of the (int) matrix
 */
void add1(PciDevState *pcidev)
{

    size_t rows = pcidev->bar2[0];
    size_t cols = pcidev->bar2[1];

    int *row;

    mlock(pcidev->addr, rows * cols * sizeof(int));

    for (size_t i = 0; i < rows; i++)
    {

        row = &((int *)pcidev->addr)[i * cols];

        for (size_t j = 0; j < cols; j++)
        {
            // printf("%d ", row[j]);
            row[j]++;
        }

        // printf("\n");
    }

    munlock(pcidev->addr, rows * cols * sizeof(int));

    clean(pcidev);

    msi_notify(&pcidev->pdev, 0);

    return;
}

/**
 * Transform a RGB matrix in greyscale
 */
void togrey(PciDevState *pcidev)
{

    size_t rows = pcidev->bar2[0];
    size_t cols = pcidev->bar2[1];

    RGB *row;

    mlock(pcidev->addr, rows * cols * sizeof(RGB));

    for (size_t i = 0; i < rows; i++)
    {

        row = &((RGB *)pcidev->addr)[i * cols];

        for (size_t j = 0; j < cols; j++)
        {

            // printf("\x1b[48;2;%d;%d;%dm  \x1b[0m", row[j].r, row[j].g, row[j].b);

            row[j].r = (77 * row[j].r + 150 * row[j].g + 29 * row[j].b) >> 8;
            row[j].g = row[j].r;
            row[j].b = row[j].r;
        }

        // printf("\n");
    }

    munlock(pcidev->addr, rows * cols * sizeof(RGB));

    clean(pcidev);

    msi_notify(&pcidev->pdev, 0);

    return;
}

/**
 * Convolute the matrix to find edges
 */
void convol(PciDevState *pcidev)
{

    size_t rows = pcidev->bar2[0];
    size_t cols = pcidev->bar2[1];

    int kernelX[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}};

    int kernelY[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1}};

    mlock(pcidev->addr, rows * cols * sizeof(RGB));

    RGB *mtr = malloc(rows * cols * sizeof(RGB));

    if (!mtr)
    {
        printf("Malloc failed\n");
        munlock(pcidev->addr, rows * cols * sizeof(RGB));
        return;
    }

    RGB *row, *row2, *row3;

    int vx, vy;

    for (size_t i = 1; i < rows - 1; i++)
    {

        vx = 0;
        vy = 0;

        row = &((RGB *)pcidev->addr)[(i - 1) * cols];
        row2 = &((RGB *)pcidev->addr)[i * cols];
        row3 = &((RGB *)pcidev->addr)[(i + 1) * cols];

        for (size_t j = 1; j < cols - 1; j++)
        {

            vx = row[j - 1].g * kernelX[0][0] +
                 row[j].g * kernelX[0][1] +
                 row[j + 1].g * kernelX[0][2] +
                 row2[j - 1].g * kernelX[1][0] +
                 row2[j].g * kernelX[1][1] +
                 row2[j + 1].g * kernelX[1][2] +
                 row3[j - 1].g * kernelX[2][0] +
                 row3[j].g * kernelX[2][1] +
                 row3[j + 1].g * kernelX[2][2];

            vy = row[j - 1].g * kernelY[0][0] +
                 row[j].g * kernelY[0][1] +
                 row[j + 1].g * kernelY[0][2] +
                 row2[j - 1].g * kernelY[1][0] +
                 row2[j].g * kernelY[1][1] +
                 row2[j + 1].g * kernelY[1][2] +
                 row3[j - 1].g * kernelY[2][0] +
                 row3[j].g * kernelY[2][1] +
                 row3[j + 1].g * kernelY[2][2];

            mtr[i * cols + j].g = (uint8_t)sqrt(vx * vx + vy * vy);

            mtr[i * cols + j].b = mtr[i * cols + j].g;
            mtr[i * cols + j].r = mtr[i * cols + j].g;

            // printf("\x1b[48;2;%d;%d;%dm  \x1b[0m", mtr[i * cols + j].b, mtr[i * cols + j].b, mtr[i * cols + j].b);
        }

        // printf("\n");
    }

    memcpy(pcidev->addr, mtr, rows * cols * sizeof(RGB));

    munlock(pcidev->addr, rows * cols * sizeof(RGB));

    // stbi_write_bmp("/home/leonardo/Scrivania/out.bmp", cols, rows, 3, mtr);

    free(mtr);

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

    if (addr + size > sizeof(pcidev->bar0) || size != 4 || addr % 4 != 0)
    {
        printf("BAR0 read invalid addr/size (addr=0x%lx size=%u)\n", addr, size);
        return 0;
    }

    val = *(uint32_t *)(pcidev->bar0 + addr);

    // printf("BAR0 read addr %lx size %x val %lx\n", addr, size, val);
    return val;
}

static void pcidev_bar0_mmio_write(void *opaque, hwaddr addr, uint64_t val, unsigned size)
{
    PciDevState *pcidev = opaque;

    if (addr + size > sizeof(pcidev->bar0) || size != 4 || addr % 4 != 0)
    {
        printf("BAR0 write invalid addr/size (addr=0x%lx size=%u)\n", addr, size);
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
    case conv:
        convol(pcidev);
        break;
    default:
        break;
    }

    // printf("BAR0 write addr %lx size %x val %lx\n", addr, size, val);
}

// Only 4 bytes access
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
        printf("BAR1 read invalid addr/size (addr=0x%lx size=%u)\n", addr, size);
        return 0;
    }
    // printf("BAR1 read pointer %lx\n", (unsigned long)pcidev->bar1);
    return pcidev->bar1;
}

static void pcidev_bar1_mmio_write(void *opaque, hwaddr addr, uint64_t val, unsigned size)
{
    PciDevState *pcidev = opaque;
    if (addr != 0 || size != sizeof(dma_addr_t))
    {
        printf("BAR1 write invalid addr/size (addr=0x%lx size=%u)\n", addr, size);
        return;
    }
    pcidev->bar1 = val;

    pcidev->size = SIZE;

    pcidev->addr = cpu_physical_memory_map(pcidev->bar1, &pcidev->size, true);

    if (!pcidev->addr)
    {
        printf("Address translation failed\n");
        return;
    }

    // printf("BAR1 write pointer %lx\n", (unsigned long)val);
}

// Only 8 bytes access
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
        printf("BAR2 read invalid addr/size (addr=%lx size=%u)\n", addr, size);
        return 0;
    }

    uint32_t val = pcidev->bar2[addr / 4];

    // printf("BAR2 read addr 0x%lx size %u val 0x%x\n", addr, size, val);
    return val;
}

static void pcidev_bar2_mmio_write(void *opaque, hwaddr addr, uint64_t val, unsigned size)
{
    PciDevState *pcidev = opaque;

    if (size != 4 || addr % 4 != 0 || addr >= sizeof(pcidev->bar2))
    {
        printf("BAR2 write invalid addr/size (addr=%lx size=%u)\n", addr, size);
        return;
    }

    pcidev->bar2[addr / 4] = val;

    // printf("BAR2 write addr 0x%lx size %u val 0x%lx\n", addr, size, val);
}

// Only 4 bytes access
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

/**
 * Used to setup an instance
 */
static void pci_pcidev_realize(PCIDevice *pdev, Error **errp)
{
    PciDevState *pcidev = PCIDEV(pdev);

    // Setup memory
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
    pcie_endpoint_cap_init(pdev, 0);

    // Power management
    pci_pm_init(pdev, 0, errp);

    // MSI
    msi_init(pdev, 0, 1, true, false, errp);

    int offset = 0x100;

    // Serial number
    pcie_dev_ser_num_init(pdev, offset, 0x123456789ABCDEF0);

    offset += 12;
}

/**
 * Called on instantiation
 */
static void pcidev_instance_init(Object *obj)
{
    return;
}

/**
 * Called on de-instantiation
 */
static void pci_pcidev_uninit(PCIDevice *pdev)
{
    PciDevState *pcidev = PCIDEV(pdev);

    cpu_physical_memory_unmap(pcidev->addr, pcidev->size, true, pcidev->size);

    return;
}

/**
 * Initialize this class
 */
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

/**
 * Register this device class
 */
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
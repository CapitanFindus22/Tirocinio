#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <./../../../../library/cmd.h>

#define V_ID 0x1234
#define D_ID 0xbeef

int major = 247;
short b2_offset = 0;

void *buf_ptr;
dma_addr_t handle;
size_t buf_size = 4096;

struct dev {

	void *ptr_bar0, *ptr_bar1;
	uint32_t *ptr_bar2;
	struct pci_dev *pdev;

} dev;

// Needed for registration
static struct pci_device_id ids[] = {

	{PCI_DEVICE(V_ID, D_ID)},
	{0, }

};
MODULE_DEVICE_TABLE(pci, ids);

static int dev_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{

	int status = pcim_enable_device(pdev);

	if (status != 0)
	{

		dev_err(&pdev->dev, "Could not enable device\n");
		return status;
	}

	dev.pdev = pdev;

	dev.ptr_bar0 = pcim_iomap(pdev, 0, pci_resource_len(pdev, 0));
	dev.ptr_bar1 = pcim_iomap(pdev, 1, pci_resource_len(pdev, 1));
	dev.ptr_bar2 = (uint32_t*) pcim_iomap(pdev, 2, pci_resource_len(pdev, 2));

	if (!dev.ptr_bar0 || !dev.ptr_bar1 || !dev.ptr_bar2)
	{
		dev_err(&pdev->dev, "Error mapping BAR(s)\n");
		return -ENODEV;
	}

	if (dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64)))
	{
		dev_err(&pdev->dev, "64-bit DMA not supported\n");
		return -EIO;
	}

	buf_ptr = dma_alloc_coherent(&pdev->dev, buf_size, &handle, GFP_KERNEL);

	printk(KERN_INFO "%llu\n", handle);

	writeq(handle,dev.ptr_bar1);

	if (pci_is_pcie(pdev))
	{
		dev_info(&pdev->dev, "Device is PCIe\n");
	}
	else
	{
		dev_warn(&pdev->dev, "Device is legacy PCI\n");
	}

	return 0;
}

static void dev_remove(struct pci_dev *pdev)
{
	dma_free_coherent(&pdev->dev, buf_size, buf_ptr, handle);

	printk("Removing Device\n");
}

static int dev_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Device aperto\n");
	return 0;
}

static long int dev_ioctl(struct file *file, unsigned command, unsigned long arg)
{
	switch (command)
	{
	case wr_func:
		iowrite32((uint32_t)arg,dev.ptr_bar0);
		break;
	case wr_args:
		if(b2_offset < 64) {
				iowrite32((uint32_t)arg, dev.ptr_bar2 + b2_offset);
				b2_offset++;
		}
		break;
	case rst_offset:
		b2_offset = 0;
		break;
	case rd_args:
		for (size_t i = 0; i < b2_offset; i++)
		{
			printk(KERN_INFO "%d at 0x%lu\n", ioread32(dev.ptr_bar2 + i), i*4);
		}
		break;
	default:
		break;
	}

	return 0;

}

static int dev_mmap(struct file *filp, struct vm_area_struct *vma)
{
    size_t size = vma->vm_end - vma->vm_start;

    // Check bounds
    if (size > buf_size) {
        return -EINVAL;
	}

	return remap_pfn_range(
		vma,
		vma->vm_start,
		handle >> PAGE_SHIFT,
		size,
		vma->vm_page_prot);
}


static struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open = dev_open,
	.unlocked_ioctl = dev_ioctl,
	.mmap = dev_mmap,
};

static struct pci_driver dev_driver = {

	.name = "custom-dev-driver",
	.probe = dev_probe,
	.remove = dev_remove,
	.id_table = ids,

};

static int __init my_driver_init(void)
{

	if (register_chrdev(major, "my_driver", &my_fops) < 0)
	{
		printk(KERN_ALERT "Registering char device failed\n");
		return -1;
	}
	if (pci_register_driver(&dev_driver))
	{
		printk(KERN_ERR "pci_register_driver failed\n");
		return -1;
	}
	return 0;
}

static void __exit my_driver_exit(void)
{

	unregister_chrdev(major, "my_driver");
	pci_unregister_driver(&dev_driver);
}

module_init(my_driver_init);
module_exit(my_driver_exit);

// Used to ignore init and exit functions
// module_pci_driver(dev_driver);

// Metadata
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Leonardo Ganzaroli");
MODULE_DESCRIPTION("Basic driver for custom device");
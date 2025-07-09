#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define V_ID 0x1234
#define D_ID 0xbeef

int major = 247;

// Needed for registration
static struct pci_device_id ids[] = {

	{PCI_DEVICE(V_ID, D_ID)},
	{0, }

};
MODULE_DEVICE_TABLE(pci, ids);

static int dev_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{

	int status;

	void *ptr_bar0, *ptr_bar1, *ptr_bar2;

	status = pcim_enable_device(pdev);

	if (status != 0)
	{

		dev_err(&pdev->dev, "Could not enable device\n");
		return status;
	}

	ptr_bar0 = pcim_iomap(pdev, 0, pci_resource_len(pdev, 0));
	ptr_bar1 = pcim_iomap(pdev, 1, pci_resource_len(pdev, 1));
	ptr_bar2 = pcim_iomap(pdev, 2, pci_resource_len(pdev, 2));

	if (!ptr_bar0 || !ptr_bar1 || !ptr_bar2)
	{
		dev_err(&pdev->dev, "Error mapping BAR(s)\n");
		return -ENODEV;
	}

	if (dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64)))
	{
		dev_err(&pdev->dev, "64-bit DMA not supported\n");
		return -EIO;
	}

	if (pci_is_pcie(pdev))
	{
		dev_info(&pdev->dev, "Device is PCIe\n");
	}
	else
	{
		dev_warn(&pdev->dev, "Device is legacy PCI\n");
	}

	iowrite32(0x2155325,ptr_bar0);
	writeq(0x255325,ptr_bar1);
	iowrite32(0x255325,ptr_bar2);

	dev_info(&pdev->dev, "%u %llu %u\n", ioread32(ptr_bar0), readq(ptr_bar1), ioread32(ptr_bar2));

	return 0;
}

static void dev_remove(struct pci_dev *pdev)
{
	printk("Removing Device\n");
}

static int dev_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Device aperto\n");
	return 0;
}


static struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open = dev_open,
};

static struct pci_driver dev_driver = {

	.name = "custom-dev-driver",
	.probe = dev_probe,
	.remove = dev_remove,
	.id_table = ids,

};

static int __init my_driver_init(void)
{

	if (pci_register_driver(&dev_driver))
	{
		printk(KERN_ERR "pci_register_driver failed\n");
		return -1;
	}

	if (register_chrdev(major, "my_driver", &my_fops) < 0)
	{
		printk(KERN_ALERT "Registering char device failed\n");
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
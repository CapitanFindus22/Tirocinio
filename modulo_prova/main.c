#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/delay.h>

#define V_ID 0x1234
#define D_ID 0xbeef

static struct pci_device_id ids[] = {

	{PCI_DEVICE(V_ID,D_ID)},
	{}

};
MODULE_DEVICE_TABLE(pci, ids);

static int dev_probe(struct pci_dev *pdev, const struct pci_device_id *id) {

	int status;

	void *ptr_bar0, *ptr_bar1;

	status = pcim_enable_device(pdev);

	if(status != 0) {

		printk(KERN_ERR "Could not enable device\n");
		return status;

	}

	ptr_bar0 = pcim_iomap(pdev, 0, pci_resource_len(pdev, 0));
	ptr_bar1 = pcim_iomap(pdev, 1, pci_resource_len(pdev, 1));

	if (!ptr_bar0 || !ptr_bar1)
	{
		printk(KERN_ERR "Error while mapping BARs\n");
		return -ENODEV;
	}

	iowrite8(15,ptr_bar1);

	printk(KERN_DEBUG "%d\n", ioread8(ptr_bar1));

	iowrite8(90,ptr_bar1+1);

	printk(KERN_DEBUG "%d\n", ioread8(ptr_bar1+1));

	return 0;

}

static void dev_remove(struct pci_dev *pdev) {

	printk(KERN_INFO "Removing device\n");

}

static struct pci_driver dev_driver = {

	.name = "custom-dev-driver",
	.probe = dev_probe,
	.remove = dev_remove,
	.id_table = ids,

};
module_pci_driver(dev_driver);

MODULE_LICENSE("GPL");
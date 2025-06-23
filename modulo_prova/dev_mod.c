#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/delay.h>

#define V_ID 0x1234
#define D_ID 0xbeef

//Needed for registration
static struct pci_device_id ids[] = {

	{PCI_DEVICE(V_ID, D_ID)},
	{}

};
MODULE_DEVICE_TABLE(pci, ids);

static int dev_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{

	int status;

	void *ptr_bar0, *ptr_bar1;

	status = pcim_enable_device(pdev);

	if (status != 0)
	{

		dev_err(&pdev->dev, "Could not enable device\n");
		return status;
	}

	ptr_bar0 = pcim_iomap(pdev, 0, pci_resource_len(pdev, 0));
	ptr_bar1 = pcim_iomap(pdev, 1, pci_resource_len(pdev, 1));

	if (!ptr_bar0 || !ptr_bar1)
	{
		dev_err(&pdev->dev, "Error mapping BARs\n");
		return -ENODEV;
	}

	//Set 
	if (dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64))) {
		dev_err(&pdev->dev, "64-bit DMA not supported\n");
		return -EIO;
	}

	if (pci_is_pcie(pdev)) {
		dev_info(&pdev->dev,  "Device is PCIe\n");
	} else {
		dev_warn(&pdev->dev,  "Device is legacy PCI\n");
	}

	dma_addr_t handle;
	dma_addr_t handle2;

	char *buff = (char *)dma_alloc_coherent(&pdev->dev, 15, &handle, GFP_KERNEL);
	char *buff2 = (char *)dma_alloc_coherent(&pdev->dev, 15, &handle2, GFP_KERNEL);

	for (size_t i = 0; i < 15; i++)
	{
		buff[i] = i;
		buff2[i] = i + 1;
	}

	writeq(handle, (__u64 *)ptr_bar1);
	iowrite8(15, ptr_bar1 + 4);

	printk(KERN_INFO "Value: %d", ioread8(ptr_bar1));

	writeq(handle2, (__u64 *)ptr_bar1);
	iowrite8(15, ptr_bar1 + 4);

	printk(KERN_INFO "Value: %d", ioread8(ptr_bar1));

	dma_free_coherent(&pdev->dev, 15, buff, handle);
	dma_free_coherent(&pdev->dev, 15, buff2, handle2);

	return 0;
}

static void dev_remove(struct pci_dev *pdev)
{
	printk("Removing Device\n");
}

static struct pci_driver dev_driver = {

	.name = "custom-dev-driver",
	.probe = dev_probe,
	.remove = dev_remove,
	.id_table = ids,

};

//Used to ignore init and exit functions
module_pci_driver(dev_driver);

//Metadata
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Leonardo Ganzaroli");
MODULE_DESCRIPTION("Basic module for custom device");
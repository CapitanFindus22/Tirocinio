#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/completion.h>
#include <./../../../../library/cmd.h>

#define V_ID 0x1234
#define D_ID 0xbeef

int major = 247;
short b2_offset = 0;

void *buf_ptr;
dma_addr_t handle;
size_t buf_size = 4096;

struct dev
{

	void *ptr_bar0, *ptr_bar1;
	uint32_t *ptr_bar2;
	struct pci_dev *pdev;

	int irq;

} dev;

static DECLARE_COMPLETION(irq_done);

/**
 * Wrapper, wait for a signal from the device
 */
#define DO_AND_WAIT(cmd)                              \
	do                                                \
	{                                                 \
		reinit_completion(&irq_done);                 \
		cmd;                                          \
		wait_for_completion_interruptible(&irq_done); \
	} while (0)

// Needed for registration
static struct pci_device_id ids[] = {

	{PCI_DEVICE(V_ID, D_ID)},
	{
		0,
	}

};
MODULE_DEVICE_TABLE(pci, ids);

/**
 * Interrupt handler
 */
static irqreturn_t my_irq_handler(int irq, void *dev_id)
{
	complete(&irq_done);
	printk(KERN_INFO "Interrupt ricevuto\n");
	return IRQ_HANDLED;
}

/**
 * Called when a compatible device is found
 */
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
	dev.ptr_bar2 = (uint32_t *)pcim_iomap(pdev, 2, pci_resource_len(pdev, 2));

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

	pci_set_master(pdev);

	buf_ptr = dma_alloc_coherent(&pdev->dev, buf_size, &handle, GFP_KERNEL);

	if (!buf_ptr)
	{
		dev_err(&pdev->dev, "allocazione fallita\n");
		return -ENOMEM;
	}

	writeq(handle, dev.ptr_bar1);

	if (pci_alloc_irq_vectors(pdev, 1, 1, PCI_IRQ_MSI) != 1)
	{
		pci_free_irq_vectors(pdev);
		dma_free_coherent(&pdev->dev, buf_size, buf_ptr, handle);
		return -ENOMSG;
	}

	dev.irq = pci_irq_vector(pdev, 0);

	if (devm_request_irq(&pdev->dev, dev.irq, my_irq_handler,
						 0, "custom-dev-driver", &dev))
	{
		dev_err(&pdev->dev, "request_irq fallita\n");
		pci_free_irq_vectors(pdev);
		dma_free_coherent(&pdev->dev, buf_size, buf_ptr, handle);
		return -EBUSY;
	}

	return 0;
}

/**
 * Called when the device is removed
 */
static void dev_remove(struct pci_dev *pdev)
{
	pci_free_irq_vectors(pdev);

	dma_free_coherent(&pdev->dev, buf_size, buf_ptr, handle);

	pci_disable_device(pdev);

	printk("Removing Device\n");
}

/**
 * Called when the virtual device is opened
 */
static int dev_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Device aperto\n");
	return 0;
}

/**
 * Ioctl implementation
 */
static long int dev_ioctl(struct file *file, unsigned command, unsigned long arg)
{
	switch (command)
	{
	case wr_func:
		DO_AND_WAIT(iowrite32((uint32_t)arg, dev.ptr_bar0));
		break;
	case wr_args:
		if (b2_offset < 64)
		{
			iowrite32((uint32_t)arg, dev.ptr_bar2 + b2_offset);
			b2_offset++;
		}
		break;
	case rst_offset:
		b2_offset = 0;
		break;
	case clr_buff:
		memset(buf_ptr, 0, buf_size);
		break;
	default:
		break;
	}

	return 0;
}

/**
 * Mmap implementation
 */
static int dev_mmap(struct file *filp, struct vm_area_struct *vma)
{
	size_t size = vma->vm_end - vma->vm_start;

	// Check bounds
	if (size > buf_size)
	{
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

/**
 * Called on module insertion
 */
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

/**
 * Called on module removal
 */
static void __exit my_driver_exit(void)
{

	unregister_chrdev(major, "my_driver");
	pci_unregister_driver(&dev_driver);
}

module_init(my_driver_init);
module_exit(my_driver_exit);

// Metadata
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Leonardo Ganzaroli");
MODULE_DESCRIPTION("Basic driver for custom device");
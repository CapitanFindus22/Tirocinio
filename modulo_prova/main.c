#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

// Major device number
static int major;

// Buffer
static char buffer[64];
static size_t buffer_pointer = 0;

// Prototypes
static int mod_open(struct inode *, struct file *);
static int mod_close(struct inode *, struct file *);
static ssize_t mod_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t mod_write(struct file *, const char __user *, size_t, loff_t *);

// File operations
static struct file_operations fops = {

	.owner = THIS_MODULE,
	.open = mod_open,
	.release = mod_close,
	.read = mod_read,
	.write = mod_write,

};

static int mod_init(void)
{

	// Create a character device
	// 0 = find free device number
	major = register_chrdev(0, "prova", &fops);

	if (major < 0)
	{
		printk(KERN_ERR "Error while loading\n");
		return major;
	}

	printk(KERN_INFO "%d\n", major);
	return 0;
}
module_init(mod_init);

static void mod_exit(void)
{
	unregister_chrdev(major, "prova");
}
module_exit(mod_exit);

// Read function
static ssize_t mod_read(struct file *File, char *user_buffer, size_t count, loff_t *offs)
{
	int to_copy, not_copied, delta;

	if (*offs >= buffer_pointer)
		return 0;

	/* Get amount of data to copy */
	to_copy = min(count, buffer_pointer);

	/* Copy data to user */
	not_copied = copy_to_user(user_buffer, buffer, to_copy);

	/* Calculate data */
	delta = to_copy - not_copied;

	*offs += delta;

	return delta;
}

// Write function
static ssize_t mod_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs)
{
	int to_copy, not_copied, delta;

	/* Get amount of data to copy */
	to_copy = min(count, sizeof(buffer));

	/* Copy data to user */
	not_copied = copy_from_user(buffer, user_buffer, to_copy);
	buffer_pointer = to_copy;

	/* Calculate data */
	delta = to_copy - not_copied;

	return delta;
}

static int mod_open(struct inode *device_file, struct file *instance)
{
	printk("dev_nr - open was called!\n");
	return 0;
}

static int mod_close(struct inode *device_file, struct file *instance)
{
	printk("dev_nr - close was called!\n");
	return 0;
}

// Metadata
MODULE_LICENSE("GPL");
MODULE_AUTHOR("franco");
MODULE_DESCRIPTION("Ciao infame");

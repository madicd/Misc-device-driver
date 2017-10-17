#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("madj");

static struct miscdevice zuehlke_device = {
	.name = "zuehlke"
};

static int __init misc_init(void)
{
	misc_register(&zuehlke_device);
	return 0;
}

static void __exit misc_exit(void)
{
	misc_deregister(&zuehlke_device);
}

module_init(misc_init);
module_exit(misc_exit);

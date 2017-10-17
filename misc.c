#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("madj");

static const char zuehlke_camp[] = "ZuehlkeCamp2017\n";
static const int zuehlke_camp_len = strlen(zuehlke_camp);

static ssize_t read_zuehlke(struct file *fd, char __user *buf, size_t len, loff_t *off) {
	return simple_read_from_buffer(buf, len, off, zuehlke_camp, zuehlke_camp_len);
}

static ssize_t write_zuehlke(struct file *fd, const char __user *buf, size_t len, loff_t *off) {
	char tmp[zuehlke_camp_len + 1];
	int inlen = simple_write_to_buffer(tmp, zuehlke_camp_len, off, buf, len);
	
	if(inlen < 0)
		return inlen;
	
	if((inlen != zuehlke_camp_len) || (inlen != len))
		return -EINVAL;

	if(strncmp(tmp, zuehlke_camp, zuehlke_camp_len) == 0)
		return inlen;
	else
		return -EINVAL;
}

static const struct file_operations fops = {
	.read = read_zuehlke,
	.write = write_zuehlke 
};

static struct miscdevice zuehlke_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "zuehlke",
	.fops = &fops,
	.mode = 0666
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

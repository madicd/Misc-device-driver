#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/slab.h>

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

struct identity {
	char name[100];
	int id;
	struct list_head list;
};

static struct list_head id_list;

static int identity_create(char *name, int id)
{
	struct identity *id_ptr;
	id_ptr = kzalloc(sizeof(struct identity), GFP_KERNEL);
	
	strcpy(id_ptr->name, name);
	id_ptr->id = id;
	list_add(&id_ptr->list, &id_list);

	return 0;
}

static struct identity* identity_find(int id)
{
	struct identity *id_ptr;
	
	list_for_each_entry(id_ptr, &id_list, list) {
		if(id_ptr->id == id)
			return id_ptr;
	}

	return NULL;
}

static void identity_destroy(int id)
{
	struct identity *id_ptr, *next;

	list_for_each_entry_safe(id_ptr, next, &id_list, list) {
		if(id_ptr->id == id) {
			list_del(&id_ptr->list);
			kzfree(id_ptr);
			return;
		}
	}
}

static void remove_all(void)
{
	struct identity *id_ptr, *next;
	
	list_for_each_entry_safe(id_ptr, next, &id_list, list) {
		list_del(&id_ptr->list);
		kzfree(id_ptr);
	}
}

static int __init misc_init(void)
{
	INIT_LIST_HEAD(&id_list);

	return misc_register(&zuehlke_device);
}

static void __exit misc_exit(void)
{
	misc_deregister(&zuehlke_device);
}

module_init(misc_init);
module_exit(misc_exit);

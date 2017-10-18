#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>

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

DECLARE_WAIT_QUEUE_HEAD(wq);

int kthreadfn(void* data)
{
	while(!kthread_should_stop()){
		wait_event_interruptible_timeout(wq, kthread_should_stop(), HZ); 
	}

	return 0;
}

static struct task_struct* task;

static int __init misc_init(void)
{
	int res;
	struct identity *temp;
		
	INIT_LIST_HEAD(&id_list);
	res = identity_create("Alice", 1);
	if(res != 0)
		goto error_adding;
	res = identity_create("Bob", 2);
	if(res != 0)
		goto error_adding;
	res = identity_create("Chuck", 3);
	if(res != 0)
		goto error_adding;
	res = identity_create("Dan", 10);
	if(res != 0)
		goto error_adding;

	temp = identity_find(42);
	if(temp != NULL)
		pr_debug("id 42 = %s\n", temp->name);
	else
		pr_debug("id 42 not found\n");	

	identity_destroy(1);
	identity_destroy(2);
	identity_destroy(3);
	identity_destroy(10);
	identity_destroy(42);

	task = kthread_run(kthreadfn, NULL, "My First Kernel Thread");

	return misc_register(&zuehlke_device);

error_adding:
	pr_debug("error adding\n");
	remove_all();
	return res;
}

static void __exit misc_exit(void)
{
	kthread_stop(task);
	remove_all();
	misc_deregister(&zuehlke_device);
}

module_init(misc_init);
module_exit(misc_exit);

// SPDX-License-Identifier: GPL-2.0
/* Original synthetic target, disposable guest only. Linux 7.0 API adapter. */
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/rcupdate.h>
#include <linux/hrtimer.h>
#include <linux/capability.h>
#include <linux/preempt.h>
struct owner {
	struct kref ref;
	struct rcu_head rcu;
	struct mutex lock;
	struct work_struct work;
	struct hrtimer timer;
	bool closed;
	unsigned generation, queued_generation, remaining, writes, worked, size;
	unsigned char data[32];
	atomic_t irq;
};
static struct owner __rcu *published[2];
static DEFINE_MUTEX(registry);
static atomic_t allocated = ATOMIC_INIT(0), freed = ATOMIC_INIT(0);
static int fail_stage;
static bool mutex_lookup;
module_param(fail_stage, int, 0400);
module_param(mutex_lookup, bool, 0400);
static void reclaim(struct rcu_head *r)
{
	struct owner *o = container_of(r, struct owner, rcu);
	atomic_inc(&freed);
	kfree(o);
}
static void release(struct kref *r)
{
	struct owner *o = container_of(r, struct owner, ref);
	call_rcu(&o->rcu, reclaim);
}
static enum hrtimer_restart tick(struct hrtimer *t)
{
	struct owner *o = container_of(t, struct owner, timer);
	if (in_hardirq())
		atomic_inc(&o->irq);
	return HRTIMER_NORESTART;
}
static void work(struct work_struct *w)
{
	struct owner *o = container_of(w, struct owner, work);
	mutex_lock(&o->lock);
	++o->worked;
	if (!o->closed && o->queued_generation == o->generation &&
	    o->remaining) {
		--o->remaining;
		kref_get(&o->ref);
		if (!schedule_work(&o->work))
			kref_put(&o->ref, release);
	}
	mutex_unlock(&o->lock);
	kref_put(&o->ref, release);
}
static struct owner *lookup(unsigned key, bool with_mutex)
{
	struct owner *o;
	if (key >= 2)
		return NULL;
	if (with_mutex) {
		mutex_lock(&registry);
		o = rcu_dereference_protected(published[key],
					      lockdep_is_held(&registry));
		if (o && !kref_get_unless_zero(&o->ref))
			o = NULL;
		mutex_unlock(&registry);
	} else {
		rcu_read_lock();
		o = rcu_dereference(published[key]);
		if (o && !kref_get_unless_zero(&o->ref))
			o = NULL;
		rcu_read_unlock();
	}
	return o;
}
static int open_owner(struct inode *i, struct file *f)
{
	struct owner *o = lookup(0, mutex_lookup);
	if (!o)
		return -ENODEV;
	mutex_lock(&o->lock);
	if (o->closed) {
		mutex_unlock(&o->lock);
		kref_put(&o->ref, release);
		return -ENODEV;
	}
	mutex_unlock(&o->lock);
	f->private_data = o;
	return nonseekable_open(i, f);
}
static int close_owner(struct inode *i, struct file *f)
{
	(void)i;
	kref_put(&((struct owner *)f->private_data)->ref, release);
	return 0;
}
/* Header version,length,flags,reserved, followed by exactly length bytes.
   v1 flags must0; v2 flags bit0 is ignorable metadata, never authority. */
static ssize_t write_owner(struct file *f, const char __user *p, size_t n,
			   loff_t *off)
{
	struct owner *o = f->private_data;
	unsigned char tmp[36];
	(void)off;
	if (n < 4 || n > sizeof tmp)
		return -EMSGSIZE;
	if (copy_from_user(tmp, p, n))
		return -EFAULT;
	if ((tmp[0] != 1 && tmp[0] != 2) || tmp[1] > 32 ||
	    n != (size_t)tmp[1] + 4 || tmp[3] || (tmp[2] & ~1) ||
	    (tmp[0] == 1 && tmp[2]))
		return -EINVAL;
	mutex_lock(&o->lock);
	if (o->closed) {
		mutex_unlock(&o->lock);
		return -ENODEV;
	}
	memcpy(o->data, tmp + 4, tmp[1]);
	o->size = tmp[1];
	++o->writes;
	o->remaining = 2;
	o->queued_generation = o->generation;
	kref_get(&o->ref);
	if (!schedule_work(&o->work))
		kref_put(&o->ref, release);
	mutex_unlock(&o->lock);
	return n;
}
static ssize_t read_owner(struct file *f, char __user *p, size_t n, loff_t *off)
{
	struct owner *o = f->private_data;
	u32 v[6];
	(void)off;
	if (n != sizeof v)
		return -EINVAL;
	mutex_lock(&o->lock);
	v[0] = o->writes;
	v[1] = o->worked;
	v[2] = o->size;
	v[3] = o->closed;
	v[4] = atomic_read(&o->irq);
	v[5] = o->size ? o->data[0] : 0;
	mutex_unlock(&o->lock);
	if (copy_to_user(p, v, sizeof v))
		return -EFAULT;
	return sizeof v;
}
static struct owner *create_owner(unsigned generation)
{
	struct owner *o = kzalloc(sizeof *o, GFP_KERNEL);
	if (!o)
		return NULL;
	atomic_inc(&allocated);
	kref_init(&o->ref);
	mutex_init(&o->lock);
	INIT_WORK(&o->work, work);
	atomic_set(&o->irq, 0);
	o->generation = generation;
	hrtimer_setup(&o->timer, tick, CLOCK_MONOTONIC, HRTIMER_MODE_REL_HARD);
	return o;
}
/* Serialize pointer replacement and closure, then drain without worker's lock. */
static int replace(unsigned key, struct owner *next)
{
	struct owner *o;
	mutex_lock(&registry);
	o = rcu_dereference_protected(published[key],
				      lockdep_is_held(&registry));
	if (next && (!o || o->generation >= UINT_MAX - 1)) {
		mutex_unlock(&registry);
		return -ENODEV;
	}
	if (o) {
		mutex_lock(&o->lock);
		o->closed = true;
		++o->generation;
		if (next)
			next->generation = o->generation;
		rcu_assign_pointer(published[key], next);
		mutex_unlock(&o->lock);
	}
	mutex_unlock(&registry);
	if (!o)
		return 0;
	synchronize_rcu();
	hrtimer_cancel(&o->timer);
	if (cancel_work_sync(&o->work))
		kref_put(&o->ref, release);
	kref_put(&o->ref, release);
	return 0;
}
static void detach(void)
{
	replace(0, NULL);
	replace(1, NULL);
}
static long control(struct file *f, unsigned cmd, unsigned long arg)
{
	struct owner *o;
	int rc;
	u32 v[4];
	(void)f;
	if (cmd != 0xB060 && cmd != 0xB061 && cmd != 0xB062)
		return -ENOTTY;
	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	if (cmd == 0xB060) {
		detach();
		return 0;
	}
	if (cmd == 0xB061) {
		o = create_owner(0);
		if (!o)
			return -ENOMEM;
		rc = replace(0, o);
		if (rc)
			kref_put(&o->ref, release);
		return rc;
	}
	if (copy_from_user(v, (void __user *)arg, sizeof v))
		return -EFAULT;
	if (v[0] >= 2 || v[1] > 1)
		return -EINVAL;
	o = lookup(v[0], v[1]);
	if (!o)
		return -ENODEV;
	mutex_lock(&o->lock);
	v[2] = o->generation;
	v[3] = o->closed;
	mutex_unlock(&o->lock);
	kref_put(&o->ref, release);
	return copy_to_user((void __user *)arg, v, sizeof v) ? -EFAULT : 0;
}
static const struct file_operations ops = {.owner = THIS_MODULE,
					   .open = open_owner,
					   .release = close_owner,
					   .write = write_owner,
					   .read = read_owner,
					   .unlocked_ioctl = control};
static struct miscdevice dev = {.minor = MISC_DYNAMIC_MINOR,
				.name = "cm_b06_owner",
				.fops = &ops,
				.mode = 0600};
static int __init start(void)
{
	struct owner *o, *second;
	int rc;
	if (fail_stage == 1)
		return -ENOMEM;
	o = create_owner(1);
	if (!o)
		return -ENOMEM;
	if (fail_stage == 2) {
		kref_put(&o->ref, release);
		rcu_barrier();
		return -ENOMEM;
	}
	second = create_owner(1);
	if (!second) {
		kref_put(&o->ref, release);
		rcu_barrier();
		return -ENOMEM;
	}
	hrtimer_start(&o->timer, ms_to_ktime(1), HRTIMER_MODE_REL_HARD);
	rc = misc_register(&dev);
	if (rc) {
		hrtimer_cancel(&o->timer);
		kref_put(&o->ref, release);
		kref_put(&second->ref, release);
		rcu_barrier();
		return rc;
	}
	rcu_assign_pointer(published[0], o);
	rcu_assign_pointer(published[1], second);
	pr_info("B06_OWNER loaded lookup=%s\n", mutex_lookup ? "mutex" : "rcu");
	return 0;
}
static void __exit stop(void)
{
	misc_deregister(&dev);
	detach();
	rcu_barrier();
	pr_info("B06_OWNER released allocated=%d freed=%d\n",
		atomic_read(&allocated), atomic_read(&freed));
	WARN_ON(atomic_read(&allocated) != atomic_read(&freed));
}
module_init(start);
module_exit(stop);
MODULE_LICENSE("GPL");

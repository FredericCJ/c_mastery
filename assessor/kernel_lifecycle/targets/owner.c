/* Original teaching module: disposable guest only. */
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
struct owner { struct kref refs; struct mutex lock; struct work_struct work;
    bool queued; size_t size; unsigned char data[32]; };
static struct owner *root;
static void destroy(struct kref *ref) { kfree(container_of(ref,struct owner,refs)); }
static void work(struct work_struct *w) {
    struct owner *o=container_of(w,struct owner,work);
    mutex_lock(&o->lock); o->queued=false; mutex_unlock(&o->lock);
    kref_put(&o->refs,destroy);
}
static int open_owner(struct inode *inode,struct file *file) {
    (void)inode; kref_get(&root->refs); file->private_data=root; return 0;
}
static int close_owner(struct inode *inode,struct file *file) {
    (void)inode; kref_put(&((struct owner *)file->private_data)->refs,destroy); return 0;
}
static ssize_t write_owner(struct file *f,const char __user *p,size_t n,loff_t *off) {
    struct owner *o=f->private_data; unsigned char tmp[32]; (void)off;
    if(n>sizeof tmp)return -EMSGSIZE;
    if(copy_from_user(tmp,p,n))return -EFAULT; /* all-or-reject record */
    mutex_lock(&o->lock); memcpy(o->data,tmp,n); o->size=n;
    if(!o->queued) { o->queued=true;kref_get(&o->refs);
        if(!schedule_work(&o->work)){o->queued=false;kref_put(&o->refs,destroy);} }
    mutex_unlock(&o->lock);return n;
}
static const struct file_operations ops={.owner=THIS_MODULE,.open=open_owner,
    .release=close_owner,.write=write_owner,.llseek=no_llseek};
static struct miscdevice dev={.minor=MISC_DYNAMIC_MINOR,.name="cm_b06_owner",.fops=&ops,.mode=0600};
static int __init owner_start(void) {int rc;root=kzalloc(sizeof *root,GFP_KERNEL);
    if(!root)return -ENOMEM;kref_init(&root->refs);mutex_init(&root->lock);
    INIT_WORK(&root->work,work);rc=misc_register(&dev);
    if(rc){kref_put(&root->refs,destroy);root=NULL;}return rc;
}
static void __exit owner_stop(void) {misc_deregister(&dev);flush_work(&root->work);
    kref_put(&root->refs,destroy);root=NULL;}
module_init(owner_start);module_exit(owner_stop);MODULE_LICENSE("GPL");

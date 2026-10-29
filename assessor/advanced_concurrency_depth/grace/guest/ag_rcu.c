// Original teaching module: load only in the supplied disposable guest.
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/kthread.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/rcupdate.h>
#include <linux/slab.h>
#include <linux/smp.h>

struct version
{
    unsigned value;
    struct rcu_head rcu;
};
static struct version __rcu *root;
static struct hrtimer timer;
static DECLARE_COMPLETION(ready);
static DECLARE_COMPLETION(reader_done);
static DECLARE_COMPLETION(reclaimed);
static DECLARE_COMPLETION(finished);
static atomic_t release_reader = ATOMIC_INIT(0), errors = ATOMIC_INIT(0),
                freed = ATOMIC_INIT(0);
static unsigned hardirq_seen, old_seen, new_seen, held_grace;
static bool pretend_early;
module_param(pretend_early, bool, 0400);

static void reclaim(struct rcu_head *r)
{
    struct version *v = container_of(r, struct version, rcu);
    atomic_inc(&freed);
    kfree(v);
    complete(&reclaimed);
}

static enum hrtimer_restart read_irq(struct hrtimer *t)
{
    struct version *v;
    u64 deadline = ktime_get_mono_fast_ns() + 250000000ULL;
    (void)t;
    hardirq_seen = in_hardirq();
    rcu_read_lock();
    v = rcu_dereference(root);
    if (!v || v->value != 1 || !rcu_is_watching())
    {
        atomic_inc(&errors);
    }
    old_seen = v ? v->value : 0;
    complete(&ready);
    while (!atomic_read(&release_reader) && ktime_get_mono_fast_ns() < deadline)
    {
        cpu_relax();
    }
    if (!atomic_read(&release_reader) || !v || v->value != 1 ||
        atomic_read(&freed))
    {
        atomic_inc(&errors);
    }
    rcu_read_unlock();
    complete(&reader_done);
    return HRTIMER_NORESTART;
}

static void start_timer(void *unused)
{
    (void)unused;
    hrtimer_start(&timer, ms_to_ktime(1), HRTIMER_MODE_REL_PINNED_HARD);
}

static int writer(void *unused)
{
    struct version *old, *next;
    (void)unused;
    next = kzalloc(sizeof *next, GFP_KERNEL);
    if (!next)
    {
        atomic_inc(&errors);
        goto done;
    }
    next->value = 2;
    if (smp_call_function_single(0, start_timer, NULL, 1) ||
        !wait_for_completion_timeout(&ready, msecs_to_jiffies(1000)))
    {
        kfree(next);
        atomic_inc(&errors);
        goto done;
    }
    old = rcu_dereference_protected(root, 1);
    rcu_assign_pointer(root, next);
    rcu_read_lock();
    next = rcu_dereference(root);
    new_seen = next ? next->value : 0;
    rcu_read_unlock();
    call_rcu(&old->rcu, reclaim);
    held_grace = !wait_for_completion_timeout(&reclaimed, msecs_to_jiffies(10));
    if (!held_grace || atomic_read(&freed))
    {
        atomic_inc(&errors);
    }
    /* Harmless negative: corrupt the observation, never free a protected object.
     */
    if (pretend_early)
    {
        held_grace = 0;
        atomic_inc(&errors);
    }
    atomic_set(&release_reader, 1);
    if (!wait_for_completion_timeout(&reader_done, msecs_to_jiffies(1000)))
    {
        atomic_inc(&errors);
    }
    if (held_grace &&
        !wait_for_completion_timeout(&reclaimed, msecs_to_jiffies(2000)))
    {
        atomic_inc(&errors);
    }
done:
    atomic_set(&release_reader, 1);
    complete(&finished);
    for (;;)
    {
        set_current_state(TASK_INTERRUPTIBLE);
        if (kthread_should_stop())
        {
            break;
        }
        schedule();
    }
    __set_current_state(TASK_RUNNING);
    return 0;
}

static void cleanup(void)
{
    struct version *v;
    atomic_set(&release_reader, 1);
    hrtimer_cancel(&timer);
    v = rcu_dereference_protected(root, 1);
    RCU_INIT_POINTER(root, NULL);
    synchronize_rcu();
    kfree(v);
    rcu_barrier();
}

static int __init start(void)
{
    struct task_struct *task;
    struct version *v;
    int result = 0;
    if (!cpu_online(0) || !cpu_online(1))
    {
        return -ENODEV;
    }
    v = kzalloc(sizeof *v, GFP_KERNEL);
    if (!v)
    {
        return -ENOMEM;
    }
    v->value = 1;
    rcu_assign_pointer(root, v);
    hrtimer_setup(&timer, read_irq, CLOCK_MONOTONIC,
                  HRTIMER_MODE_REL_PINNED_HARD);
    task = kthread_create(writer, NULL, "ag-rcu-writer");
    if (IS_ERR(task))
    {
        cleanup();
        return PTR_ERR(task);
    }
    kthread_bind(task, 1);
    wake_up_process(task);
    if (!wait_for_completion_timeout(&finished, msecs_to_jiffies(5000)))
    {
        atomic_inc(&errors);
        atomic_set(&release_reader, 1);
    }
    kthread_stop(task);
    hrtimer_cancel(&timer);
    rcu_barrier();
    if (atomic_read(&errors) || !hardirq_seen || old_seen != 1 ||
        new_seen != 2 || !held_grace || atomic_read(&freed) != 1)
    {
        result = -EINVAL;
    }
    if (pretend_early)
    {
        pr_info("AG_RCU_NEGATIVE_DETECTED errors=%d held=%u\n",
                atomic_read(&errors), held_grace);
    }
    else
    {
        pr_info("AG_RCU_OBSERVATION hardirq=%u old=%u new=%u held_grace=%u "
                "freed=%d errors=%d\n",
                hardirq_seen, old_seen, new_seen, held_grace,
                atomic_read(&freed), atomic_read(&errors));
    }
    if (result)
    {
        cleanup();
    }
    return result;
}

static void __exit stop(void)
{
    cleanup();
    pr_info("AG_RCU_UNLOADED callbacks_drained=1\n");
}

module_init(start);
module_exit(stop);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Original isolated RCU read-side hardirq exercise");

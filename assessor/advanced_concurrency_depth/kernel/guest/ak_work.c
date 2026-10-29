// SPDX-License-Identifier: GPL-2.0
/* Original synthetic work/requeue target: disposable guest only. */
#include <linux/completion.h>
#include <linux/hrtimer.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
static DECLARE_COMPLETION(request);
static DECLARE_COMPLETION(enough);
static DEFINE_MUTEX(gate);
static struct work_struct work;
static struct hrtimer timer;
static struct task_struct *producer;
static bool closed;
static unsigned runs, admitted, requests, after_close;
static atomic_t timer_fired = ATOMIC_INIT(0);
static int false_drain;
module_param(false_drain, int, 0400);
static enum hrtimer_restart tick(struct hrtimer *t)
{
	(void)t;
	atomic_inc(&timer_fired);
	return HRTIMER_NORESTART;
}
static void worker(struct work_struct *w)
{
	void *p;
	(void)w;
	/* Ordinary worker may sleep/allocate, but receives no borrowed user pointer.
	 */
	p = kmalloc(32, GFP_KERNEL);
	if (!p)
		return;
	kfree(p);
	mutex_lock(&gate);
	++runs;
	if (!closed && runs < 16) {
		++requests;
		complete(&request);
	}
	if (runs >= 16)
		complete(&enough);
	mutex_unlock(&gate);
}
static int producing(void *unused)
{
	(void)unused;
	for (;;) {
		if (wait_for_completion_interruptible(&request))
			continue;
		if (kthread_should_stop())
			break;
		mutex_lock(&gate);
		if (!closed) {
			if (schedule_work(&work))
				++admitted;
		} else {
			++after_close;
			mutex_unlock(&gate);
			for (;;) {
				set_current_state(TASK_INTERRUPTIBLE);
				if (kthread_should_stop())
					break;
				schedule();
			}
			__set_current_state(TASK_RUNNING);
			break;
		}
		mutex_unlock(&gate);
	}
	return 0;
}
static void drain(void)
{
	mutex_lock(&gate);
	closed = true;
	mutex_unlock(&gate);
	complete(&request);
	if (producer) {
		kthread_stop(producer);
		producer = NULL;
	}
	cancel_work_sync(&work);
	hrtimer_cancel(&timer);
}
static int __init start(void)
{
	unsigned before;
	int rc = 0;
	INIT_WORK(&work, worker);
	hrtimer_setup(&timer, tick, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	producer = kthread_run(producing, NULL, "cm_ak_producer");
	if (IS_ERR(producer)) {
		rc = PTR_ERR(producer);
		producer = NULL;
		return rc;
	}
	hrtimer_start(&timer, ms_to_ktime(10000), HRTIMER_MODE_REL);
	complete(&request);
	if (!wait_for_completion_timeout(&enough, msecs_to_jiffies(2000))) {
		drain();
		return -ETIMEDOUT;
	}
	/* Separate external producer, not self-requeue: close gate then join before
	 * cancel. */
	mutex_lock(&gate);
	before = runs;
	mutex_unlock(&gate);
	drain();
	if (false_drain) {
		pr_info("AK_WORK_NEGATIVE no_racing_enqueue_required=1\n");
		return -EINVAL;
	}
	if (before != 16 || runs != 16 || admitted != 16 || requests != 15 ||
	    atomic_read(&timer_fired)) {
		pr_err("AK_WORK_FAIL runs=%u admitted=%u requests=%u\n", runs,
		       admitted, requests);
		return -EINVAL;
	}
	pr_info("AK_WORK_PASS runs=16 external_requeues=15 gate_closed=1 "
		"producer_joined=1 work_drained=1 timer_drained=1\n");
	return 0;
}
static void __exit stop(void)
{
	drain();
	pr_info("AK_WORK_UNLOADED retained_callbacks=0\n");
}
module_init(start);
module_exit(stop);
MODULE_LICENSE("GPL");

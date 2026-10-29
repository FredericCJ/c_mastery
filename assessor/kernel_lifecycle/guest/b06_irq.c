// SPDX-License-Identifier: GPL-2.0
/* Original synthetic software IRQ source; load only inside disposable guest. */
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/hrtimer.h>
#include <linux/completion.h>
#include <linux/mutex.h>
#include <linux/preempt.h>
struct payload {
	struct completion done;
	struct mutex lock;
	struct hrtimer timer;
	unsigned primary, thread, task;
};
static struct device *device;
static struct payload *state;
static int irq_number = -1;
static bool requested;
static int bad_hardirq, fail_stage;
module_param(bad_hardirq, int, 0400);
module_param(fail_stage, int, 0400);
static void noop(struct irq_data *d)
{
	(void)d;
}
static struct irq_chip chip = {.name = "cm-b06-original",
			       .irq_mask = noop,
			       .irq_unmask = noop,
			       .irq_ack = noop};
static void managed_release(void *p)
{
	struct payload *s = p;
	pr_info("B06_MANAGED_RELEASE primary=%u thread=%u task=%u\n",
		s->primary, s->thread, s->task);
}
static irqreturn_t primary(int n, void *p)
{
	struct payload *s = p;
	(void)n;
	if (!in_hardirq())
		return IRQ_NONE;
	s->primary++;
	if (bad_hardirq) {
		pr_info("B06_EXPECTED_ATOMIC_DIAGNOSTIC\n");
		might_sleep();
	}
	return IRQ_WAKE_THREAD;
}
static irqreturn_t threaded(int n, void *p)
{
	struct payload *s = p;
	void *owned;
	(void)n;
	mutex_lock(&s->lock);
	if (in_hardirq() || irqs_disabled()) {
		mutex_unlock(&s->lock);
		return IRQ_NONE;
	}
	owned = kmalloc(32, GFP_KERNEL);
	if (owned) {
		s->thread++;
		kfree(owned);
	}
	mutex_unlock(&s->lock);
	complete(&s->done);
	return IRQ_HANDLED;
}
static enum hrtimer_restart fire(struct hrtimer *t)
{
	(void)t;
	generic_handle_irq((unsigned)irq_number);
	return HRTIMER_NORESTART;
}
static void cleanup(void)
{
	if (state)
		hrtimer_cancel(&state->timer);
	if (requested) {
		free_irq((unsigned)irq_number, state);
		requested = false;
	}
	if (irq_number >= 0) {
		irq_set_chip_and_handler((unsigned)irq_number, NULL, NULL);
		irq_free_desc((unsigned)irq_number);
		irq_number = -1;
	}
	if (device) {
		root_device_unregister(device);
		device = NULL;
	}
	state = NULL;
}
static int __init start(void)
{
	int rc;
	device = root_device_register("cm_b06_irq");
	if (IS_ERR(device)) {
		rc = PTR_ERR(device);
		device = NULL;
		return rc;
	}
	state = devm_kzalloc(device, sizeof *state, GFP_KERNEL);
	if (!state) {
		cleanup();
		return -ENOMEM;
	}
	init_completion(&state->done);
	mutex_init(&state->lock);
	hrtimer_setup(&state->timer, fire, CLOCK_MONOTONIC,
		      HRTIMER_MODE_REL_HARD);
	rc = devm_add_action_or_reset(device, managed_release, state);
	if (rc) {
		cleanup();
		return rc;
	}
	if (fail_stage == 1) {
		cleanup();
		return -ENOMEM;
	}
	mutex_lock(&state->lock);
	state->task++;
	mutex_unlock(&state->lock);
	irq_number = irq_alloc_descs(-1, 0, 1, NUMA_NO_NODE);
	if (irq_number < 0) {
		rc = irq_number;
		cleanup();
		return rc;
	}
	irq_set_chip_and_handler((unsigned)irq_number, &chip,
				 handle_simple_irq);
	irq_clear_status_flags((unsigned)irq_number,
			       IRQ_NOREQUEST | IRQ_NOAUTOEN);
	rc = request_threaded_irq((unsigned)irq_number, primary, threaded,
				  IRQF_ONESHOT, "cm-b06-original", state);
	if (rc) {
		cleanup();
		return rc;
	}
	requested = true;
	if (fail_stage == 2) {
		cleanup();
		return -ENOMEM;
	}
	hrtimer_start(&state->timer, ms_to_ktime(1), HRTIMER_MODE_REL_HARD);
	if (!wait_for_completion_timeout(&state->done,
					 msecs_to_jiffies(1000)) ||
	    state->primary != 1 || state->thread != 1) {
		cleanup();
		return -ETIMEDOUT;
	} /* The self-owned descriptor pins this module while requested: quiesce this one-shot source before init returns, otherwise unload would depend on its own exit callback. */
	hrtimer_cancel(&state->timer);
	free_irq((unsigned)irq_number, state);
	requested = false;
	pr_info("B06_IRQ_PASS task=1 hardirq=1 threaded=1 bad=%d\n",
		bad_hardirq);
	return 0;
}
static void __exit stop(void)
{
	cleanup();
}
module_init(start);
module_exit(stop);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(
	"Original disposable-guest context and managed-lifetime exercise");

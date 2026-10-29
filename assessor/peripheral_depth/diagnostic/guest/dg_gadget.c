// SPDX-License-Identifier: GPL-2.0
/* Original diagnostic teaching gadget. Load only inside its networkless guest.
 */
#include "diagnostic.h"
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/usb/gadget.h>
#ifndef DG_GUEST_MUTANT
#define DG_GUEST_MUTANT 0
#endif
static struct usb_ep *input, *output;
static struct usb_request *ctrl, *rx, *tx;
static DGSession session;
static DEFINE_SPINLOCK(lock);
static bool active, tx_owned, in_submit, rx_owned;
static unsigned accepted, rejected, inline_completions, completed, cancelled,
	rx_cancelled;
static struct usb_endpoint_descriptor in_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_IN | 1,
	.bmAttributes = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize = cpu_to_le16(8)};
static struct usb_endpoint_descriptor out_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 1,
	.bmAttributes = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize = cpu_to_le16(8)};
static const u8 device[18] = {18,  1, 0, 2, 0, 0, 0, 8, 255,
			      255, 1, 0, 0, 1, 0, 0, 0, 1};
static u8 config[32] = {9, 2, 32, 0, 1, 1, 0, 128, 50, 9, 4, 0,	  0, 2, 255, 0,
			0, 0, 7,  5, 1, 2, 8, 0,   0,  7, 5, 129, 2, 8, 0,   0};
static void control_done(struct usb_ep *ep, struct usb_request *r)
{
	(void)ep;
	(void)r;
}
static void transmit_done(struct usb_ep *ep, struct usb_request *r)
{
	unsigned long flags;
	(void)ep;
	spin_lock_irqsave(&lock, flags);
	tx_owned = false;
	completed++;
	if (in_submit)
		inline_completions++;
	if (r->status == -ECONNRESET || r->status == -ESHUTDOWN)
		cancelled++;
	spin_unlock_irqrestore(&lock, flags);
}
static void receive_done(struct usb_ep *ep, struct usb_request *r)
{
	DGMessage message;
	size_t n;
	int rc;
	unsigned long flags;
	bool send = false;
	spin_lock_irqsave(&lock, flags);
	rx_owned = false;
	if (r->status) {
		if (r->status == -ECONNRESET || r->status == -ESHUTDOWN)
			rx_cancelled++;
		spin_unlock_irqrestore(&lock, flags);
		return;
	}
	if (!active) {
		spin_unlock_irqrestore(&lock, flags);
		return;
	}
	rc = tx_owned ? DG_BUSY
		      : dg_accept(&session, r->buf, r->actual, &message);
	if (!rc && !tx_owned) {
		rc = dg_encode(&message, tx->buf, 36, &n);
		if (!rc) {
			tx->length = n;
			tx->zero = 1;
			tx_owned = true;
			in_submit = true;
			send = true;
			accepted++;
		}
	} else
		rejected++;
	spin_unlock_irqrestore(&lock, flags);
	if (send) {
		/* Do not hold our lock: inspected dummy controller can give back inline. */
		rc = usb_ep_queue(input, tx, GFP_ATOMIC);
		spin_lock_irqsave(&lock, flags);
		in_submit = false;
		if (rc)
			tx_owned = false;
		spin_unlock_irqrestore(&lock, flags);
	}
	r->length = 40; /* >36 and multiple of MPS: detect oversized packets. */
	spin_lock_irqsave(&lock, flags);
	rx_owned = true;
	spin_unlock_irqrestore(&lock, flags);
	rc = usb_ep_queue(ep, r, GFP_ATOMIC);
	if (rc) {
		spin_lock_irqsave(&lock, flags);
		rx_owned = false;
		spin_unlock_irqrestore(&lock, flags);
		pr_err("DG_GADGET_RX_QUEUE %d\n", rc);
	}
}
static void disable(void)
{
	unsigned long flags;
	spin_lock_irqsave(&lock, flags);
	active = false;
	spin_unlock_irqrestore(&lock, flags);
	if (output && output->enabled)
		usb_ep_disable(output);
	if (input && input->enabled)
		usb_ep_disable(input);
}
static int enable(void)
{
	int rc;
	disable();
	input->desc = &in_desc;
	output->desc = &out_desc;
	rc = usb_ep_enable(input);
	if (rc)
		return rc;
	rc = usb_ep_enable(output);
	if (rc) {
		usb_ep_disable(input);
		return rc;
	}
	active = true;
	rx_owned = true;
	rx->length = 40;
	rc = usb_ep_queue(output, rx, GFP_ATOMIC);
	if (rc) {
		rx_owned = false;
		disable();
	}
	return rc;
}
static int setup(struct usb_gadget *g, const struct usb_ctrlrequest *q)
{
	unsigned value = le16_to_cpu(q->wValue), index = le16_to_cpu(q->wIndex),
		 length = le16_to_cpu(q->wLength);
	int n = -EOPNOTSUPP;
	if (q->bRequestType == USB_DIR_IN &&
	    q->bRequest == USB_REQ_GET_DESCRIPTOR && !index && !(value & 255)) {
		if (value == USB_DT_DEVICE << 8) {
			memcpy(ctrl->buf, device, sizeof device);
			n = sizeof device;
		}
		if (value == USB_DT_CONFIG << 8) {
			memcpy(ctrl->buf, config, sizeof config);
			n = sizeof config;
		}
	} else if (q->bRequestType == 0 &&
		   q->bRequest == USB_REQ_SET_CONFIGURATION && value <= 1 &&
		   !index && !length) {
		if (value)
			n = enable();
		else {
			disable();
			n = 0;
		}
	} else if (q->bRequestType == USB_DIR_IN &&
		   q->bRequest == USB_REQ_GET_CONFIGURATION && !value &&
		   !index && length == 1) {
		*(u8 *)ctrl->buf = active;
		n = 1;
	} else if (q->bRequestType == (USB_DIR_IN | USB_RECIP_INTERFACE) &&
		   q->bRequest == USB_REQ_GET_INTERFACE && !value && !index &&
		   length == 1 && active) {
		*(u8 *)ctrl->buf = 0;
		n = 1;
	} else if (q->bRequestType == USB_RECIP_INTERFACE &&
		   q->bRequest == USB_REQ_SET_INTERFACE && !value && !index &&
		   !length && active)
		n = enable();
	/* Original vendor request exposes synthetic counters, never host memory. */
	else if (q->bRequestType == 0xc0 && q->bRequest == 0x51 && !value &&
		 !index && length == 24) {
		u32 snapshot[6];
		unsigned long flags;
		spin_lock_irqsave(&lock, flags);
		snapshot[0] = cpu_to_le32(accepted);
		snapshot[1] = cpu_to_le32(rejected);
		snapshot[2] = cpu_to_le32(inline_completions);
		snapshot[3] = cpu_to_le32(completed);
		snapshot[4] = cpu_to_le32(cancelled);
		snapshot[5] = cpu_to_le32(rx_cancelled);
		spin_unlock_irqrestore(&lock, flags);
		memcpy(ctrl->buf, snapshot, sizeof snapshot);
		n = sizeof snapshot;
	} else if (q->bRequestType == 0x40 && q->bRequest == 0x52 && !value &&
		   !index && !length && active) {
		n = usb_ep_dequeue(output, rx);
	} else if (q->bRequestType == 0x40 && q->bRequest == 0x53 && !value &&
		   !index && !length && active) {
		unsigned long flags;
		bool owned;
		spin_lock_irqsave(&lock, flags);
		owned = rx_owned;
		if (!owned)
			rx_owned = true;
		spin_unlock_irqrestore(&lock, flags);
		if (owned)
			return -EBUSY;
		rx->length = 40;
		n = usb_ep_queue(output, rx, GFP_ATOMIC);
		if (n) {
			spin_lock_irqsave(&lock, flags);
			rx_owned = false;
			spin_unlock_irqrestore(&lock, flags);
		}
	}
	if (n < 0)
		return n;
	ctrl->length = min_t(unsigned, n, length);
	ctrl->zero = (n < length && n % 8 == 0);
	return usb_ep_queue(g->ep0, ctrl, GFP_ATOMIC);
}
static struct usb_request *allocate(struct usb_ep *ep, size_t size,
				    void (*done)(struct usb_ep *,
						 struct usb_request *))
{
	struct usb_request *r = usb_ep_alloc_request(ep, GFP_KERNEL);
	if (!r)
		return NULL;
	r->buf = kmalloc(size, GFP_KERNEL);
	if (!r->buf) {
		usb_ep_free_request(ep, r);
		return NULL;
	}
	r->complete = done;
	return r;
}
static void release(struct usb_ep *ep, struct usb_request **r)
{
	if (*r) {
		kfree((*r)->buf);
		usb_ep_free_request(ep, *r);
		*r = NULL;
	}
}
static int bind(struct usb_gadget *g, struct usb_gadget_driver *driver)
{
	(void)driver;
	input = usb_ep_autoconfig(g, &in_desc);
	output = usb_ep_autoconfig(g, &out_desc);
	if (!input || !output)
		return -ENODEV;
	/* Autoconfiguration chooses FS bulk64. Restore our advertised legal MPS8. */
	if (DG_GUEST_MUTANT != 1) {
		in_desc.wMaxPacketSize = cpu_to_le16(8);
		out_desc.wMaxPacketSize = cpu_to_le16(8);
	}
	config[20] = out_desc.bEndpointAddress;
	config[27] = in_desc.bEndpointAddress;
	ctrl = allocate(g->ep0, 64, control_done);
	rx = allocate(output, 40, receive_done);
	tx = allocate(input, 36, transmit_done);
	if (!ctrl || !rx || !tx) {
		release(g->ep0, &ctrl);
		release(output, &rx);
		release(input, &tx);
		return -ENOMEM;
	}
	pr_info("DG_GADGET_BOUND in=%02x out=%02x\n", in_desc.bEndpointAddress,
		out_desc.bEndpointAddress);
	return 0;
}
static void disconnect(struct usb_gadget *g)
{
	(void)g;
	disable();
}
static void unbind(struct usb_gadget *g)
{
	disable();
	release(g->ep0, &ctrl);
	release(output, &rx);
	release(input, &tx);
	pr_info("DG_GADGET_UNBOUND accepted=%u rejected=%u inline=%u completed=%u "
		"cancelled=%u\n",
		accepted, rejected, inline_completions, completed, cancelled);
}
static struct usb_gadget_driver driver = {
	.function = "C Mastery diagnostic",
	.max_speed = USB_SPEED_FULL,
	.bind = bind,
	.unbind = unbind,
	.setup = setup,
	.disconnect = disconnect,
	.reset = disconnect,
	.driver = {.name = "dg_gadget", .owner = THIS_MODULE}};
static int __init start(void)
{
	return usb_gadget_register_driver(&driver);
}
static void __exit stop(void)
{
	usb_gadget_unregister_driver(&driver);
}
module_init(start);
module_exit(stop);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(
	"Original C Mastery diagnostic teaching gadget for an isolated guest");

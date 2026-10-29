// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/usb/gadget.h>
/* Learner-owned adapter: deliberately refuse binding until implemented. */
static int bind(struct usb_gadget *g, struct usb_gadget_driver *d)
{
	(void)g;
	(void)d;
	pr_info("DG_GADGET_STUB\n");
	return -ENOSYS;
}
static void unbind(struct usb_gadget *g)
{
	(void)g;
}
static void reset(struct usb_gadget *g)
{
	(void)g;
}
static int setup(struct usb_gadget *g, const struct usb_ctrlrequest *q)
{
	(void)g;
	(void)q;
	return -EOPNOTSUPP;
}
static struct usb_gadget_driver driver = {
	.function = "C Mastery unfinished diagnostic",
	.max_speed = USB_SPEED_FULL,
	.bind = bind,
	.unbind = unbind,
	.reset = reset,
	.disconnect = reset,
	.setup = setup,
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
	"Unfinished original C Mastery diagnostic gadget learner scaffold");

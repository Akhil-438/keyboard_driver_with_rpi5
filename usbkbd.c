#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/input.h>
#include <linux/slab.h>

#define USB_KBD_MINOR_BASE 0

#ifndef USB_INTERFACE_CLASS_HID
#define USB_INTERFACE_CLASS_HID            3
#endif
#ifndef USB_INTERFACE_SUBCLASS_BOOT
#define USB_INTERFACE_SUBCLASS_BOOT        1
#endif
#ifndef USB_INTERFACE_PROTOCOL_KEYBOARD
#define USB_INTERFACE_PROTOCOL_KEYBOARD    1
#endif


struct usb_kbd {
    struct usb_device *usbdev;
    struct input_dev *input;
    struct urb *irq;
    unsigned char *new;
    dma_addr_t new_dma;
};

static void usb_kbd_irq(struct urb *urb)
{
    struct usb_kbd *kbd = urb->context;
    int i;

    if (urb->status)
        return;

    for (i = 2; i < 8; i++) {
        if (kbd->new[i])
            input_report_key(kbd->input, kbd->new[i], 1);
        else
            input_report_key(kbd->input, kbd->new[i], 0);
    }
    input_sync(kbd->input);

    usb_submit_urb(urb, GFP_ATOMIC);
}

static int usb_kbd_probe(struct usb_interface *iface, const struct usb_device_id *id)
{
    struct usb_device *dev = interface_to_usbdev(iface);
    struct usb_kbd *kbd;
    struct input_dev *input_dev;
    struct usb_endpoint_descriptor *endpoint;
    int pipe, maxp, error = -ENOMEM;

    endpoint = &iface->cur_altsetting->endpoint[0].desc;

    kbd = kzalloc(sizeof(struct usb_kbd), GFP_KERNEL);
    if (!kbd)
        return -ENOMEM;

    input_dev = input_allocate_device();
    if (!input_dev)
        goto fail1;

    kbd->usbdev = dev;
    kbd->input = input_dev;

    input_dev->name = dev->product ? dev->product : "USB Keyboard";
    input_dev->phys = "usb/input0";

    input_dev->id.bustype = BUS_USB;
    input_dev->id.vendor  = le16_to_cpu(dev->descriptor.idVendor);
    input_dev->id.product = le16_to_cpu(dev->descriptor.idProduct);
    input_dev->id.version = le16_to_cpu(dev->descriptor.bcdDevice);

    input_set_capability(input_dev, EV_KEY, KEY_A);
    input_set_capability(input_dev, EV_KEY, KEY_B);
    input_set_capability(input_dev, EV_KEY, KEY_C);
    input_set_capability(input_dev, EV_KEY, KEY_D);
    input_set_capability(input_dev, EV_KEY, KEY_E);
    input_set_capability(input_dev, EV_KEY, KEY_F);
    input_set_capability(input_dev, EV_KEY, KEY_G);
    input_set_capability(input_dev, EV_KEY, KEY_H);
    input_set_capability(input_dev, EV_KEY, KEY_I);
    input_set_capability(input_dev, EV_KEY, KEY_J);
    input_set_capability(input_dev, EV_KEY, KEY_K);
    input_set_capability(input_dev, EV_KEY, KEY_L);
    input_set_capability(input_dev, EV_KEY, KEY_M);
    input_set_capability(input_dev, EV_KEY, KEY_N);
    input_set_capability(input_dev, EV_KEY, KEY_O);
    input_set_capability(input_dev, EV_KEY, KEY_P);
    input_set_capability(input_dev, EV_KEY, KEY_Q);
    input_set_capability(input_dev, EV_KEY, KEY_R);
    input_set_capability(input_dev, EV_KEY, KEY_S);
    input_set_capability(input_dev, EV_KEY, KEY_T);
    input_set_capability(input_dev, EV_KEY, KEY_U);
    input_set_capability(input_dev, EV_KEY, KEY_V);
    input_set_capability(input_dev, EV_KEY, KEY_W);
    input_set_capability(input_dev, EV_KEY, KEY_X);
    input_set_capability(input_dev, EV_KEY, KEY_Y);
    input_set_capability(input_dev, EV_KEY, KEY_Z);

    input_set_capability(input_dev, EV_KEY, KEY_1);
    input_set_capability(input_dev, EV_KEY, KEY_2);
    input_set_capability(input_dev, EV_KEY, KEY_3);
    input_set_capability(input_dev, EV_KEY, KEY_4);
    input_set_capability(input_dev, EV_KEY, KEY_5);
    input_set_capability(input_dev, EV_KEY, KEY_6);
    input_set_capability(input_dev, EV_KEY, KEY_7);
    input_set_capability(input_dev, EV_KEY, KEY_8);
    input_set_capability(input_dev, EV_KEY, KEY_9);
    input_set_capability(input_dev, EV_KEY, KEY_0);

    pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
    maxp = usb_maxpacket(dev, pipe);

    kbd->new = usb_alloc_coherent(dev, 8, GFP_ATOMIC, &kbd->new_dma);
    if (!kbd->new)
        goto fail2;

    kbd->irq = usb_alloc_urb(0, GFP_KERNEL);
    if (!kbd->irq)
        goto fail3;

    usb_fill_int_urb(kbd->irq, dev, pipe, kbd->new, (maxp > 8 ? 8 : maxp),
                     usb_kbd_irq, kbd, endpoint->bInterval);
    kbd->irq->transfer_dma = kbd->new_dma;
    kbd->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

    error = input_register_device(kbd->input);
    if (error)
        goto fail4;

    usb_set_intfdata(iface, kbd);
    usb_submit_urb(kbd->irq, GFP_KERNEL);

    dev_info(&iface->dev, "USB Keyboard connected\n");
    return 0;

fail4:
    usb_free_urb(kbd->irq);
fail3:
    usb_free_coherent(dev, 8, kbd->new, kbd->new_dma);
fail2:
    input_free_device(input_dev);
fail1:
    kfree(kbd);
    return error;
}

static void usb_kbd_disconnect(struct usb_interface *iface)
{
    struct usb_kbd *kbd = usb_get_intfdata(iface);

    usb_set_intfdata(iface, NULL);
    if (kbd) {
        usb_kill_urb(kbd->irq);
        input_unregister_device(kbd->input);
        usb_free_urb(kbd->irq);
        usb_free_coherent(kbd->usbdev, 8, kbd->new, kbd->new_dma);
        kfree(kbd);
    }
    dev_info(&iface->dev, "USB Keyboard disconnected\n");
}

static const struct usb_device_id usb_kbd_id_table[] = {
    { USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID,
                         USB_INTERFACE_SUBCLASS_BOOT,
                         USB_INTERFACE_PROTOCOL_KEYBOARD) },
    { } /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, usb_kbd_id_table);

static struct usb_driver usb_kbd_driver = {
    .name = "usbkbd_custom",
    .id_table = usb_kbd_id_table,
    .probe = usb_kbd_probe,
    .disconnect = usb_kbd_disconnect,
};

module_usb_driver(usb_kbd_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Custom USB Keyboard Driver");
MODULE_AUTHOR("Bhanu + ChatGPT");

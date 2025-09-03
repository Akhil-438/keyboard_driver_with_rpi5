## USB Keyboard Driver for Raspberry Pi (Linux Kernel 6.x)

This project implements a **custom USB keyboard driver** for Raspberry Pi 5 running Linux 6.x.  
It is a learning-oriented kernel module that demonstrates how to handle USB HID devices, register with the Linux input subsystem, and provide basic keyboard functionality.

---

## ✨ Features
- Implements **USB HID Boot Protocol keyboard driver**.
- Logs **key press events** via `dmesg`.
- Registers with the **Linux input subsystem** (`/dev/input/event*`).
- Supports **CapsLock / NumLock LED control** (demo).
- Provides manual **bind/unbind** for testing with a physical keyboard.
- Educational example for **Linux Device Driver (LDD)** development.



If you want a visual diagram, here’s a suggested flow:


\[USB Keyboard] → \[usbkbd.ko] → \[Linux Input Subsystem] → \[User Space / Terminal]



<img width="1024" height="1536" alt="ChatGPT Image Sep 3, 2025, 11_16_22 AM" src="https://github.com/user-attachments/assets/9b0ca615-e45e-4451-a507-5b6c71106636" />

## ⚙️ Requirements
- Raspberry Pi 5 (or other ARM64 board)  
- Linux Kernel `6.12.x` (with headers installed)  
- USB keyboard for testing  
- Basic knowledge of kernel module compilation  

Install kernel headers:

```bash
sudo apt update
sudo apt install raspberrypi-kernel-headers build-essential
```

## 🔨 Build

Clone the repo and build:

```bash
make
```

Clean build:

```bash
make clean
```

---

## ▶️ Usage

1. **Insert the driver**:

```bash
sudo insmod usbkbd.ko
```

2. **Check logs**:

```bash
dmesg | tail -20
```

3. **Find your keyboard device**:

```bash
ls /sys/bus/usb/devices/
```

(Example: `3-1:1.0`)

4. **Unbind from default driver** (`usbhid`):

```bash
echo -n "3-1:1.0" | sudo tee /sys/bus/usb/drivers/usbhid/unbind
```

5. **Bind to this driver**:

```bash
echo -n "3-1:1.0" | sudo tee /sys/bus/usb/drivers/usbkbd/bind
```

6. **Test keypresses**:

```bash
dmesg -w
```

---

## 🛑 Recovery (if input stops working)

If your keyboard stops responding:

```bash
sudo rmmod usbkbd
echo -n "3-1:1.0" | sudo tee /sys/bus/usb/drivers/usbhid/bind
```

---

## 📚 Notes

* This driver is **for educational purposes only**.
* It is **not a full replacement** for the production `usbhid` driver.
* Useful for practicing **Linux USB and input subsystem driver development**.

---

## 📜 License

MIT License

````

---

👉 I can also generate a **PNG diagram** (nice, professional block diagram) that you can upload to your repo (`docs/driver_arch.png`) and reference in the README like:

```markdown
![Driver Architecture](docs/driver_arch.png)
````

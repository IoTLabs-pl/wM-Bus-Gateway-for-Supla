# wM-Bus Gateway

This repository contains the source code for the [**wM-Bus Gateway**](http://github.com/IoTLabs-pl/wM-Bus-Gateway) allowing for communication with [Supla](https://supla.org).

<p align="center">
    <img src="https://iotlabs.pl/wp-content/uploads/2025/05/P1010329.jpg" alt="wM-Bus Gateway" width="30%" />
    <img src="https://iotlabs.pl/wp-content/uploads/2025/11/ad2small.jpg" alt="wM-Bus Gateway" width="30%" />
    <img src="https://iotlabs.pl/wp-content/uploads/2025/05/4_total_energy2.jpg" alt="wM-Bus Gateway" width="30%" />
</p>

---

## ℹ️ General Info

> ⚠️ **Warning:**  
> **Do not power on the device without the antenna connected** – this may damage the radio module.

- 🟦 The device has two buttons:
  - **Top button** – used to switch pages on the display, entering configuration mode and factory reset of device.
  - **Bottom button** – used to soft reset the device.

---

## 🏠 Recommended Integration Workflow

### 📶 Wi-Fi Configuration

1. **Power up** the device with a USB cable. You may use your computer or a USB power adapter.
2. Connect to the device's Wi-Fi network named `WM-BUS-GATEWAY-xxxxxx`.

   > 💡 **Tip 1:**  
   >  Device's web interface IP and network SSID are presented on the display connection page.

   > 💡 **Tip 2:**  
   >  If you are using a mobile device, you may need to disable the mobile data connection to access the device's web interface.

3. Open a web browser and go to `http://191.168.4.1`
4. Specify your **Wi-Fi network credentials** (SSID and password) in the form.
5. **(Optional)** If you do not have your wM-Bus meters configured yet, you may also set **Config Mode** option to `ALWAYS ON` to be able to reach the device's web interface after it connects to your Wi-Fi network. Its local IP address will be shown on the display connection page.
6. Click **Save & Restart**. Device will reboot and connect to your Wi-Fi network.

### ⏱️ Meters Configuration

1. Open the device's web interface. The **wM-Bus Meters** list shows your meters first, then the meters the device hears, strongest signal first, each with its detected driver. Meters send their readings every few seconds to a few minutes, so give it a while and reload the page.

   > 💡 **Tip:**  
   > Find your meter by the number printed on it. Click its **decoder** link to analyze its last datagram in the [IoTLabs decoder](https://iotlabs.pl/decoder), e.g. to check your decryption key.

2. Switch on your meter. If your meter is encrypted, enter its key in hex-string format.

   > 💡 **Tip:**  
   > If you don't know your meter decryption key, you may try to use `00000000000000000000000000000000` (32 zeroes) as a key.

3. Press **Save & Restart** button to save the configuration and restart the device.  
   The device picks the readings for the Supla channel itself: the meter total for water, gas and heat meters; energy, power, voltage, current and frequency (per phase, if the meter sends them) for electricity meters. After the next datagram from your meter, the list shows these readings under the meter, or the decoding error, e.g. `decryption_failed` for a wrong key.  
   After a few seconds, the device should connect to **Supla Cloud** with the new configuration and you should see the device in your Supla app.
   > 💡 **Tip:**  
   > All other configuration (disabling non-used phases for one-phase meters, setting price per usage, setting units of measurement, etc.) can be done in the **Supla Cloud**.

---

## ❓ FAQ

### How to disable built-in LED?
You can disable the built-in LED in the device configuration page by setting the `LED Mode` option to `OFF`. With `ON` option, the LED will blink for any received wM-Bus datagram. With `ON METER MATCH`, the LED will blink only for datagrams matching the configured meter IDs.

### How to enter config mode?
To enter the configuration mode, press and hold the **top button** longer than 5 but less than 12 seconds. The device will start LED blink. SSID and IP address of the device will be displayed on the screen. You may also command the device to enter configuration mode through **Supla Cloud**. 

### How to factory reset the device?
To factory reset the device, press and hold the **top button** for more than 12 seconds. The device will restart with all user-specified data erased. You will need to reconfigure the device from scratch.

### How to update the firmware?
A simple web flasher based on [ESP Web Tools](https://esphome.github.io/esp-web-tools/) is available at: [https://iotlabs.pl/tools](https://iotlabs.pl/tools). It will allow you to change device firmware to any version available on Github.

---

## 🌐 More Information

More info about the device can be found at:  
👉 [https://iotlabs.pl/wm-bus-gateway/](https://iotlabs.pl/wm-bus-gateway/)
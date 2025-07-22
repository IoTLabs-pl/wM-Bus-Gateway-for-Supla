# wM-Bus Gateway

This repository contains the source code for the [**wM-Bus Gateway**](http://github.com/IoTLabs-pl/wM-Bus-Gateway) allowing for communication with [Supla](https://supla.org).

<p align="center">
    <img src="https://iotlabs.pl/wp-content/uploads/2025/05/IMG_1327.jpg" alt="wM-Bus Gateway" width="30%" />
    <img src="https://iotlabs.pl/wp-content/uploads/2025/05/IMG_1338.jpg" alt="wM-Bus Gateway" width="30%" />
    <img src="https://iotlabs.pl/wp-content/uploads/2025/05/IMG_1337.jpg" alt="wM-Bus Gateway" width="30%" />
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

1. Connect your device to the computer (or any other device able to communicate with USB-Serial port) using a USB cable.
2. Open your serial console (terminal) and connect to device with `115200 8N1` settings.
   > 💡 **Tip 1:**  
   > Dedicated drivers may be required for your operating system to recognize the device.
   
   > 💡 **Tip 2:**  
   > If you don't have a serial console installed you can use [Web Flasher](https://iotlabs.pl/tools) or [ESP Web Tools](https://esphome.github.io/esp-web-tools/). Use the **Logs & Console** feature from the popup menu. If you don't see device logs in the console, reset device by pressing the **bottom button** on the device.
3. Wait for the device to receive the wM-Bus datagram from your meter. You should see logs like this:
   ```
   [16:29:10][W][main:057]: Meter ID: 12345678...
   [16:29:10][W][main:060]: Frame: https://wmbusmeters.org/analyze/be44ed1...
   ```
4. Click or copy and paste the link from the logs to your browser to analyze the received datagram. At [wmbusmeters.org](https://wmbusmeters.org/analyze/) you will be able to check your decryption key (if your meter is encrypted) and see all **data fields available** to decode from the datagram.

5. In the device configuration page, configure the meter by setting appropriate `ID`, `Driver` and `Key` in hex-string format (if your meter is encrypted) and click **Save**. After page reload, the device will display an extended form based on the selected driver media type.

   > 💡 **Tip:**  
   > If you don't know your meter decryption key, you may try to use `00000000000000000000000000000000` (32 zeroes) as a key.

6. **For flow meters**:  
   In the form's `Impulse Counter` field, type the name of the field you want to use as a Supla channel value, for example `total_m3`.

   **For electricity meters**:  
   In the form's additional fields, specify names of the wM-Bus telegram fields you want to use as data for specific Supla channel attributes. For example, you may use `total_energy_consumption_kwh` for *Forward Active Energy* field and `current_power_consumption_kw` for *Active Power* field.
    > 💡 **Tip 1:**  
    > Typically, your wM-Bus Meter does not specify all fields needed for full Supla channel attributes (if you want all of them to be available, google for **Zamel MEW-01**), so most of the fields will be empty.

    > 💡 **Tip 2:**  
    > If your meter is three-phase, you may use `%d` as a placeholder in the field name. For example, you may use `voltage_at_phase_%d` in the *Voltage* field, and it will be automatically replaced with `voltage_at_phase_1`, `voltage_at_phase_2`, and `voltage_at_phase_3` for each phase.

7. Press **Save & Restart** button to save the configuration and restart the device.  
   After a few seconds, the device should connect to **Supla Cloud** with the new configuration and you should see the device in your Supla app. 
   > 💡 **Tip 1:**  
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
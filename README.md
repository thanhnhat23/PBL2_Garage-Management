# Garage Management System (C++ OOP Project)

## 📘 Overview

**Garage Management System** is a comprehensive bus fleet management application developed in **C++** with two versions:

- **Console Version** - Command-line interface (works on any system, no installation needed)
- **Qt GUI Version** - Modern graphical interface (requires Qt Framework)

The system supports managing **routes, buses, drivers, trips, tickets**, and **booking/canceling tickets**.
All data is stored in `.txt` files in the `Data/` folder.

---

## 🎯 Key Features

- **Route Management**: Add, delete, search routes
- **Bus Management**: Manage vehicles, brands, seats
- **Driver Management**: Add, delete, search drivers
- **Trip Management**: Create, delete, view trips
- **Ticket Management**: Book tickets, cancel tickets, view history
- **Statistics**: View revenue reports, seat occupancy rates

---

## 🚀 Installation and Running Guide

### ⭐ Option 1: Run Console Version (RECOMMENDED)

**Console version requires no installation, runs anywhere!**

#### Requirements:
- Windows 10/11 or Linux/macOS
- MinGW Compiler (usually pre-installed)

#### How to Run:

**Manual Build**
```powershell
g++ main.cpp Function/*.cpp -o main
./main
```

---

### Option 2: Run Qt GUI Version (Beautiful Interface)

**Modern interface with Admin Panel and User Panel**

#### Requirements:
- **Qt Framework 6.x** (free download)
- **MinGW Compiler** (comes with Qt)
- **PowerShell** (Windows) or **Bash** (Linux/macOS)

---

## Step 1: Download and Install Qt

### Windows:

1. Visit: **https://www.qt.io/download**
2. Choose **Qt Online Installer** (free)
3. Register Qt account (free, optional)
4. Install these components:
   - **Qt 6.10.1** (or newer version)
   - **MinGW 13.1** (GCC compiler)
   - **Qt Creator** (optional, IDE for coding)

**Note**: Remember the installation path (e.g., `D:\Qt\` or `C:\Qt\`)

### Linux (Ubuntu/Debian):

```bash
sudo apt-get update
sudo apt-get install -y qt6-base-dev qt6-svg-dev build-essential mingw-w64
```

### macOS:

```bash
brew install qt@6 gcc
```

---

## Step 2: Build the Application

### Windows (PowerShell):

```powershell
.\build_qt.ps1
```

**If you get "Qt not found" error**, edit `build_qt.ps1`:
- Open the file with Notepad
- Find the line `"D:\Install\QT\6.10.1\mingw_64\bin"`
- Replace with your Qt path (e.g., `"C:\Qt\6.10.1\mingw_64\bin"`)

### Linux/macOS (Bash):

```bash
cd path/to/PBL2
chmod +x build_qt.sh
./build_qt.sh
```

**Result:** Executable created at `build/release/GarageManagement.exe` (Windows) or `build/release/GarageManagement` (Linux/macOS)

---

## Step 3: Run the Application

### Windows:

```powershell
.\run_qt.ps1
```

### Linux/macOS:

```bash
./run_qt.sh
```

### Or run directly:

**Windows:**
```powershell
.\build\release\GarageManagement.exe
```

**Linux/macOS:**
```bash
./build/release/GarageManagement
```

---

## 🔐 Demo Accounts

### Admin Account
- **Username**: admin
- **Password**: admin123
- **Permissions**: Full system management

---

## 👥 Project Information

- **Project Name**: Garage Management System
- **Type**: OOP Project - PBL2
- **Language**: C++17
- **University**: Danang University of Technology (DUT)
- **Developer**: Thanh Nhat & Van Truong
- **Supervisor**: Dao Thi Tuyet Hoa

---



# 🚌 Bus Ticket Management System (C++ OOP Project)

## 📘 Overview
This project is a **console-based Bus Ticket Management System** written in **C++**, designed to manage routes, buses, drivers, tickets, and bookings for a transport company.  
It supports **searching, filtering, booking, and canceling tickets** through a simple text-based interface.

All data are stored in `.txt` files under the `Data/` folder, making it lightweight and easily editable.

## 🧩 Features

### 1. Route Management
- Display all available routes.
- Search routes by name or keyword (case-insensitive).
- Example: search `da nang - hue`.

### 2. Bus Management
- List all available **bus brands**.
- Show buses belonging to a specific brand.
- View **driver information** and **seat status** for a chosen bus.
- Seat status displayed as:
  - `Full` — seat is booked.
  - `Empty` — seat available.

### 3. Ticket Management
- Filter tickets by **brand name**.
- Show all tickets for a specific brand including:
  - Ticket ID, seat number, bus type, passenger name, phone, price, booking date, payment method, route, trip time, and bus name.
- Integrates with trips, buses, and routes to show complete details.

### 4. Booking / Cancel Ticket
#### Booking
- Book a new ticket with:
  - Passenger name & phone number.
  - Route (`From` → `To`).
  - Seat type (`VIP` / `Standard`).
  - Payment method (`Chuyển khoản`, `Tiền mặt`, or `ZaloPay`).
- The system automatically:
  - Finds available trips and empty seats.
  - Creates a new `TKxxx.txt` file entry.
  - Marks the selected seat as full (`1`) in the `Seat` file.

#### Canceling
- Cancel a ticket using passenger **name** and **phone number**.
- The system:
  - Locates the correct `TKxxx.txt` file.
  - Removes the ticket record.
  - Updates seat status back to empty (`0`).

## ⚙️ How It Works

### Data Loading
Each class (e.g., `Brand`, `Bus`, `Route`) includes a static function `fromCSV()` that converts a CSV line into an object instance.
```cpp
vector<Brand> brands = loadData("Data/Brand.txt", Brand::fromCSV);
```
The helper function `loadData()`:
- Reads a file line-by-line.
- Skips empty lines.
- Returns a `vector<T>` of loaded objects.

Menu System
The main menu is built using an infinite loop:
```cpp
while (true) {
    cout << "===== MAIN MENU =====\n";
    cout << "1. Manage Routes\n";
    cout << "2. Manage Buses\n";
    cout << "3. Manage Tickets\n";
    cout << "4. Booking/Cancel Ticket\n";
    cout << "0. Exit\n";
    ...
}
```
Each option triggers a specific function (`menuRoute`, `menuBus`, `etc.`) implementing that module’s logic.

- Helper Utilities
  - `toLowerStr()` → converts input to lowercase (for case-insensitive searching).
  - `splitCSV()` → splits each CSV line into vector<string> using , as a delimiter.

💾 Example Data Format

Data/Route.txt
```
R001,Da Nang,Hue,100
R002,Hue,Quang Tri,80
```
Data/Seat/B001.txt
```
S001,1,0
S002,2,1
```
Format: `SeatID, SeatNumber, Status (0 = empty, 1 = full)`
Data/Ticket/TK001.txt
```
TK001,TR001,B001,2,Nguyen Van A,0912345678,150000,2025-10-29 20:00,Chuyen khoan
```
🧠 Key Concepts Used
- Object-Oriented Programming (OOP)
  - Each entity (Bus, Trip, Ticket...) represented by its own class.
- Template Functions
  - loadData<T>() uses templates to load various object types from file.
- File I/O
  - Uses ifstream and ofstream for reading/writing .txt data files.
- String Manipulation
  - stringstream, transform, and find() for flexible search/filter.
- Formatted Output
  - iomanip (setw, left) for clean table-like display.

🧑‍💻 Usage Compile
```powershell
g++ main.cpp Function/*.cpp -o main
```
Run
```powershell
./main
```
Requirements: `C++17 or higher`

Windows OS (uses system("cls") and system("pause"))

For Linux/macOS users:
Replace cls with clear and pause with read -p `"Press enter to continue"`

👥 Contributors:
- Developer: Thanh Nhat & Van Truong
- Supervisor: Dao Thi Tuyet Hoa
- Institution: Danang University of Technology (DUT)
- Project Type: PBL2 — Object-Oriented Programming

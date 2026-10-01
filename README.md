# Airport Management System

A desktop airport-operations management application built in **C** using **GTK 3**. It helps manage flights, runway allocation, crew scheduling, real-time disruptions, and operational reports through a graphical interface.

## Features

- Role-based access for administrators, flight schedulers, crew schedulers, and viewers
- Add, modify, search, and delete flight records
- Flight prioritization for emergency, international, and domestic flights
- Runway assignment based on availability and flight type
- Crew scheduling with qualifications, duty-time limits, and rest-time checks
- Real-time operational events:
  - Weather delays
  - Emergency landings
  - Flight cancellations
  - Flight rescheduling
- Flight, runway, and crew status reports
- Local data storage using `.dat` files
- In-app notifications for important operational updates

## Technologies Used

- C
- GTK 3
- GCC
- File handling for local data persistence

## Build and Run

### Prerequisites

- GCC compiler
- GTK 3 development libraries
- `pkg-config`

### Linux / macOS

```bash
gcc airport_management.c -o airport_management $(pkg-config --cflags --libs gtk+-3.0)
./airport_management
```

## Project File

```text
airport_management.c   # Main source code
```

## Author

Avantheka Sreenivasan

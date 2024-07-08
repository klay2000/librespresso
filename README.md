# Librespresso
An open-source "smart" espresso machine.

## Architecture
The basic architecture of the system is such that a backend and frontend run on an SOC that communicates with a microcontroller running the firmware over UART using a fairly simple protocol. The microcontroller is responsible for controlling the hardware and the SOC is responsible for the UI and high-level control.

## Firmware
The firmware is written in C using freeRTOS and is responsible for controlling the hardware of the espresso machine. 

## Backend
The backend is written in Python and runs on the SOC. It is responsible for communicating with the microcontroller over UART and providing a REST API for the frontend to interact with.

## Frontend
The frontend is written in React and runs on the SOC. It is responsible for providing a UI for the user to interact with.
# Phone Detection System using ESP32

## Overview

This project implements a phone detection system using an ESP32. The system combines metal detection and Bluetooth Low Energy scanning to estimate the probability that a mobile phone is present nearby. The ESP32 hosts a web server that displays the detection status and probability in real time.

## Features

* Detection of nearby Bluetooth devices using BLE scanning
* Metal detection using a digital input signal
* Probability estimation of phone presence based on sensor conditions
* ESP32 creates its own WiFi hotspot
* Real-time web dashboard accessible from a browser
* Color-based status display for detection probability

## Detection Logic

The system calculates the probability of a phone being present based on the following conditions:

| Condition                                    | Probability |
| -------------------------------------------- | ----------- |
| No metal detected                            | 0%          |
| Metal detected only                          | 50%         |
| Metal detected and Bluetooth device detected | 80%         |

## Hardware Requirements

* ESP32 (ESP-WROOM-32)
* Metal detector module with digital output
* Power supply for ESP32
* Connecting wires

## Software Requirements

* Arduino IDE
* ESP32 board package for Arduino
* Libraries:

  * WiFi
  * WebServer
  * BLEDevice
  * BLEScan

## Setup

1. Upload the code to the ESP32 using Arduino IDE.
2. Power the ESP32.
3. Connect a phone or computer to the WiFi hotspot created by the ESP32.
4. Open a browser and navigate to the IP address shown in the serial monitor (usually 192.168.4.1).

## Web Interface

The ESP32 hosts a simple web page that updates automatically and displays:

* Metal detection status
* Bluetooth device detection status
* Probability of phone presence

## Applications

This system can be used for experimental phone detection in restricted areas such as classrooms, examination halls, or security monitoring environments.


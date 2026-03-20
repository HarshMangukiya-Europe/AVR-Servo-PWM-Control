# 🤖 AVR Servo PWM Control

> Servo motor control via analog potentiometer input using AVR 
> Timer/Counter PWM modes and ADC on Arduino Nano.

---

## 📋 Overview
A potentiometer on **A0** controls a servo motor angle from **0° to 180°**
proportionally. Uses **3 timers simultaneously** — all via direct AVR 
register control, no Arduino libraries.

---

## ⚙️ Features
- **Timer1** — Fast PWM for servo position (50Hz, 1ms–5ms pulse)
- **Timer0** — CTC interrupt for LED flashing when voltage > 2.5V
- **Timer2** — Fast PWM for LED breathing/fading effect
- ADC controls servo position in real-time
- Breathing effect activates when potentiometer is stable

---

## 🔧 Hardware
- Arduino Nano (ATmega328P)
- Servo motor (M1) on D9
- Potentiometer (R1) on A0
- LED on D13 and D3
- Breadboard + resistors

---

## 🛠️ Tools Used
- Language: **AVR-C** (no Arduino libraries)
- IDE: **AVR Studio / Microchip Studio**
- Board: **Arduino Nano**

---

## 🎥 Demo Video
[![Watch on YouTube](https://img.shields.io/badge/YouTube-Watch%20Demo-red?logo=youtube)](https://youtube.com/shorts/mz2UqN2fCoY?si=hgcoVk_e5MuaU_jE)

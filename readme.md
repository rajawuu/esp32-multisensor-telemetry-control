# 🌐 Multisensor-Based Indoor Disaster Monitoring System

<p align="center">
  <b>Smart Indoor Disaster Monitoring and Early Detection System</b>
  <br>
  An ESP32-based monitoring system designed to observe environmental conditions and detect potential disaster indicators inside buildings.
</p>

---

## 📌 About The Project

**Multisensor-Based Indoor Disaster Monitoring System** merupakan proyek pengembangan sistem pemantauan bencana di dalam ruangan dengan memanfaatkan beberapa sensor yang terintegrasi menggunakan mikrokontroler ESP32.

Sistem ini dirancang untuk memantau kondisi lingkungan dan mendeteksi beberapa indikator yang berpotensi berkaitan dengan situasi darurat, seperti getaran gempa, perubahan suhu dan kelembapan, keberadaan asap atau gas, serta kemungkinan adanya reruntuhan atau hambatan di sekitar area pemantauan.

Data dari masing-masing sensor dikumpulkan oleh ESP32, kemudian dikirimkan dan divisualisasikan melalui sistem monitoring. Pengembangan proyek dilakukan secara bertahap, mulai dari pembacaan multisensor melalui Serial Monitor, komunikasi data secara wireless, hingga implementasi Graphical User Interface (GUI) menggunakan Node-RED Dashboard.

## 🎯 Project Objectives

- Mengintegrasikan beberapa sensor untuk memantau kondisi lingkungan di dalam ruangan.
- Mendeteksi perubahan percepatan atau getaran yang berpotensi berkaitan dengan aktivitas gempa.
- Memantau perubahan suhu, kelembapan, serta keberadaan asap atau gas sebagai indikator potensi kebakaran.
- Mengidentifikasi perubahan jarak yang dapat mengindikasikan adanya reruntuhan atau hambatan.
- Mengirimkan data sensor secara wireless menggunakan ESP32.
- Menampilkan data sensor secara real-time melalui dashboard monitoring.
- Mempermudah pemantauan kondisi sensor dan informasi sistem melalui satu antarmuka.

## 🔧 Hardware Components

| Component | Function |
|---|---|
| ESP32 | Main microcontroller for sensor acquisition and data communication. |
| AHT10 | Measures temperature and humidity to monitor environmental changes that may indicate fire-related conditions. |
| MQ-2 | Detects smoke and combustible gases. |
| HC-SR04 | Measures distance to identify changes in distance that may indicate debris or obstacles. |
| MMA845X | Measures acceleration on X, Y, and Z axes to detect vibration and shaking. |

## 🏗️ System Architecture

The system consists of several sensor modules connected to an ESP32 microcontroller. Sensor readings are collected and processed before being transmitted to the monitoring interface.

```text
       SENSOR MODULES
              |
    +---------+---------+
    |         |         |
   AHT10     MQ-2     HC-SR04
    |         |         |
    +---------+---------+
              |
           MMA845X
              |
              v
          ESP32 MCU
              |
       Wireless Data
              |
              v
         Node-RED
              |
              v
       GUI DASHBOARD
              |
              v
     Real-Time Monitoring
```

## 📊 Sensor Monitoring

### 1. AHT10 – Temperature & Humidity Monitoring
Monitors room temperature and relative humidity. Changes in these environmental parameters can provide supporting information for identifying abnormal room conditions.

### 2. MQ-2 – Smoke & Gas Detection
Detects the presence of smoke and combustible gases. The sensor provides analog readings that can be monitored to observe changes in air conditions.

### 3. HC-SR04 – Ultrasonic Distance Monitoring
Measures the distance between the sensor and nearby objects. Changes in measured distance can be used to identify obstacles or potential debris that may appear in the monitored area.

### 4. MMA845X – Vibration & Acceleration Monitoring
Measures acceleration along three axes (X, Y, and Z). Changes in acceleration can be analyzed to identify vibration or shaking that may be associated with seismic activity.

## 🚀 Project Development Stages

This project is developed through three main stages:

### Stage 1 – Multisensor Integration & Serial Monitor

**Report 1: Multisensor and Serial Monitor**

The initial stage focuses on integrating multiple sensors with the ESP32 microcontroller. Sensor readings are collected and displayed through the Arduino Serial Monitor to verify that each sensor can operate and provide data correctly.

**Main implementation:**
- Sensor integration with ESP32.
- Reading data from multiple sensors.
- Displaying sensor values through Serial Monitor.
- Initial testing of sensor functionality.

### Stage 2 – Wireless Data Communication

**Report 2: Wireless Data**

The second stage focuses on developing wireless communication to transmit sensor readings from the ESP32 to an external monitoring system. This stage expands the system beyond local serial monitoring.

**Main implementation:**
- Wireless data transmission.
- Sending sensor readings from ESP32.
- Receiving and processing transmitted data.
- Testing communication performance and reliability.

### Stage 3 – GUI & Monitoring

**Report 3: GUI Monitoring**

The latest development stage focuses on building a graphical monitoring interface using Node-RED Dashboard. Sensor data is visualized through gauges, charts, and status indicators to make system monitoring easier and more intuitive.

**Main implementation:**
- Node-RED Dashboard development.
- Real-time sensor data visualization.
- Gauge indicators and time-series charts.
- Sensor and actuator status monitoring.
- Communication performance monitoring, including payload, latency, throughput, and data loss.

## 🖥️ Dashboard Preview

The monitoring dashboard is developed using **Node-RED Dashboard** with the project identity:

**ELKA PENS – Smart Telemetry & Control**

It provides a centralized interface for observing sensor readings and monitoring the operational status of each module.

<!-- Add your dashboard screenshot here -->
<!-- Example: ![Dashboard Preview](docs/images/dashboard-preview.png) -->

## 🛠️ Technologies Used

- **ESP32** – Microcontroller and sensor data acquisition.
- **Arduino IDE** – Embedded programming and firmware development.
- **Node-RED** – Data processing and dashboard development.
- **Node-RED Dashboard** – Graphical user interface for monitoring.
- **Wireless Communication** – Data transmission between ESP32 and the monitoring platform.

## 📈 Communication Performance

During the GUI monitoring stage, communication performance was evaluated using six testing samples.

| Parameter | Average Result |
|---|---:|
| Payload | 135.5 bytes |
| Latency | 82.33 ms |
| Throughput | 0.5420 kbit/s |
| Data Loss | 0.10% |

These measurements provide an overview of the communication performance during the testing period.

## 🔮 Future Development

Potential improvements for the next development stages include:

- Implementing threshold-based warnings for abnormal sensor readings.
- Adding notification mechanisms for emergency conditions.
- Improving sensor data visualization and historical logging.
- Conducting longer-duration system stability tests.
- Integrating multiple sensor readings into a more comprehensive indoor disaster monitoring system.

## 👥 Project Information

**Institution:** Politeknik Elektronika Negeri Surabaya (PENS)  
**Project:** Multisensor-Based Indoor Disaster Monitoring System  
**Development Platform:** ESP32 & Node-RED  
**Project Category:** Embedded Systems | Internet of Things | Disaster Monitoring

---

<p align="center">
  <b>ELKA PENS – Smart Telemetry & Control</b>
  <br>
  <i>Integrating Sensors, Connectivity, and Visualization for Smarter Indoor Monitoring.</i>
</p>

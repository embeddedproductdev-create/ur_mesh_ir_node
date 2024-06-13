
# IR BLE Mesh AC Controller
The scope of the project is to develop a BLE Mesh based Universal AC controller which can be used to control Air Conditioners remotely

- Project Start Date   : 
- Project End Data     : 

## Project Members
- **Project Manager** : K.N.Singh
- **Project Lead**    : Kulasekaran
- **Project Members** : Umamaheswari, Adhikesavan

# High level overview
- An open source IR Library for Transmission and reception of IR Signals.
- Uses LTE for MQTT communication
- Uses BLE Mesh network
- Custom MQTT packets designed according to Project requirements. More details about this in Software documentation. (link provided below)
- Uses IDF-FreeRTOS for implementing Multi-processing application.
- Uses on-board EEPROM for flash storage

## Hardware Details
- [Schematics]()

## Software Documentation and other Helper documents
- Software Documentation : [Link to Software Documentation]()
- MCU : ESP32-S3 [Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- LTE : Quectel's EC200U [Datasheet]()
- [AT commands manual]()
- [TCP/IP command manual]()
- [MQTT commands manual]()

## Software Dependencies
- VS Code IDE
- VS Code ESP-IDF Extension
- ESP-IDF v5.0 or above
- Arduino as ESP component. Check this tutorial : [Adding Arduino as ESP IDF component](https://www.youtube.com/watch?v=hHzGX-K6lmo&pp=ygUfQWRkaW5nIGFyZHVpbm8gYXMgZXNwIGNvbXBvbmVudA%3D%3D)
- IRremoteESP8266 Library
- FreeRTOS
- BLE Mesh
- MQTT

## Instructions to Build and compile the project
1. Install VS Code in your PC. [Link to VS Code](https://code.visualstudio.com/download)
2. Once Installed, click on extensions and download **ESP-IDF** extension.
   1. Press **ctrl+shift+P** to open command palette. In that, choose **ESP-IDF: Configure ESP-IDF extension**.
   2. ESP-IDF configuration window opens
   3. Choose **Express** option.
      1. Let `Download server` be **Github**
      2. Choose ESP-IDF version that's greater than 5.1 for arduion component to be compatible with the build.
      3. Set the `ESP-IDF directory (IDF_PATH)` as **"C:\Espressif"**
      4. Set the `ESP-IDF Tools directory` as **"C:\Espressif\frameworks\tools"**
      5. Click on Install and wait for atleast 0.5 hour to 1 hour depending on your network speed.
3. Clone this repository to your system
4. Open the Project directory in VS Code
5. Press **ctrl+shift+P** to open command palette. In that, choose **ESP-IDF: Add Arduino ESP32 as ESP-IDF component**
   1. Now, this process may sometimes fail with error code 128. I'v faced this myself many times. I'm not sure of the reason. With any effort, make this process succeed.
   2. After successful completion of the process, a folder called **components\arduino** will have been generated in the project directory. This enables us to use functions that are generally available in arduion environment in ESP environment. 
6. **NOTE**: 
   1. The next step is required only if you are creating the project from scratch. By default the IRremoteESP8266 library being used in this project is not available in the components/arduino folder. So, we need to manually download the library, add the source and header files at locations and make necessary changes to the CMakeLists.txt inside the components/arduino folder in order for the compiler to be able to pick up these custom files.
   2. It's not mandatory that the custom libraries also needs to reside inside the arduino/components folder. But since it already has a structure, I placed inside it to avoid too much head scratching in trying to fix bug that'll rise upon compiling. If you can figure out a better and efficient way, kudos to you.

## Steps to include custom library files
These steps will guide you in including a custom library and using it in this project. I've taken the IRremoteESP8266 library as an example. It is available [here](https://github.com/crankyoldgit/IRremoteESP8266)
1. Download the library as a zip file.
2. Unzip it and you will find a folder named **IRremoteESP8266-master**. Copy this entire folder to `"Project directory/components/arduino/libraries"` path.
3. Inside the `Project directory/components/arduino/` there will be a file named **CMakeLists.txt**. 
   1. **NOTE**: The following instruction may not apply to you exactly because it may differ based on the ESP-IDF version that you are going to use in future. I started this project with using **`ESP-IDF v5.1.2`**. But the overall procedure will be same. The compiler needs to know that there is a file in this location that it can use during compilation. Let's continue with the procedure ...
   1. In this file, there will be a line **set(includedirs**. Inside that, include the following: `libraries/IRremoteESP8266/src`.

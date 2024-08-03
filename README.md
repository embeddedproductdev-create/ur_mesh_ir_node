<h1 align="center">IR BLE Mesh AC Controller</h1>

The scope of the project is to develop a BLE Mesh based Universal AC controller which can be used to control Air Conditioners remotely.

- Project Start Date   : 14th December, 2023
- Project End Date     :

## Project Members

- **Project Manager** : Saravana Perumal, Narendhra Singh
- **Project Lead**    : Kulasekaran
- **Project Members** : Umamaheswari, Adhikesavan

## Releases

<center>

| No | Release Version | Release date | Author | Changes | Link To Binaries |
|----|-----------------|--------------|--------|---------|------------------|
|1|0.6|24.04.2024|Kulasekaran|1) Fixed Teaching mode bug - Umamaheswari|[Ver 0.6](https://qmaxltd-my.sharepoint.com/:f:/g/personal/embedded_qmaxsys_com/EsAfWCuAF5pDgIHuPiZqN0sB0QvNj8S_XqIxa7qbzuJ3xw?e=lQr8X0)|
|2|0.7|15.06.2024|Kulasekaran|1) Long run LTE issue workaround with Rebooting|[Ver 0.7](https://qmaxltd-my.sharepoint.com/:f:/g/personal/embedded_qmaxsys_com/ElOpzTQT0UdEvAXS_M6xHUoBCObxUryYEZ7gFOrPOhTN-Q?e=Iybtim)|
|3|0.8.1|28.06.2024|Kulasekaran|1) Location string length increased from 20 to 30<br>2) Temperature Data ACK converted into Heartbeat ACK <br> 3) Minimum value for Heartbeat publish configuration increased from 5 to 10<br>3) Added Purple LED indication when sending out and IR signal<br>4) Added Teaching mode start Packets for both Gwy/Node<br>5) Added Teaching Mode End ACK for both Gwy and Node<br>6) Added Debug Info packets for both Gwy and Node<br>7) Error code values changed<br>8) LED indication change for Teaching mode and AC Remote configuration mode<br>9) Added BootUp LED indication<br>10) Modified Button Press Logics|[Ver 0.8.1](https://qmaxltd-my.sharepoint.com/:f:/g/personal/embedded_qmaxsys_com/EmEgsRgmN0pBqarVdK6FILoBhknFlkaPrFwbDWWqYjniaw?e=blzBsU)|
|4|0.8.5|08.07.2024|Kulasekaran|1) Fixed issue in AC not getting controlled<br>2) Fix for NodeSerNo not found in Prov ACK<br>3) Fix for unable to perform Teaching Mode|[Ver 0.8.5](http://git.qmaxsys.com/Unimation/c01230-ir-ble-mesh-ac-controller-gwy/archive/v0.8.5.zip)|
|5|0.8.6|10.07.2024|Kulasekaran|1) Fix for Manual Control ACK not appearing in v0.8.5<br>2)Fix for Gwy reboot issue<br>3)Fix for Node MacId missing in Node Prov Ack<br>4)Fix for Ambient temperature being 0 in Node Heartbeat ACK|[Ver 0.8.6](http://git.qmaxsys.com/Unimation/c01230-ir-ble-mesh-ac-controller-gwy/archive/v0.8.6.zip)|
|6|0.8.7|19.07.2024|Kulasekaran|1) LED indication for unsupported remotes during AC remote configuration process<br>2) NODE_ALREADY_PROV error code removed<br>3) Removed suspend & resume method to avoid assert fail during AC control<br>4) Removed separate thread used for sending IR commands|[Ver 0.8.7](http://git.qmaxsys.com/Unimation/c01230-ir-ble-mesh-ac-controller-gwy/releases/tag/v0.8.7)|
|7|0.8.8|23.07.2024|Kulasekaran|1) Added JSON_PACKET_INVALID errorcode<br>2) Default Heartbeat increased from 10s to 300s<br>3) OTA related changes<br>4) Fixed Teaching Mode End ACK not received from Node|[Ver 0.8.8](http://git.qmaxsys.com/Unimation/c01230-ir-ble-mesh-ac-controller-gwy/releases/tag/v0.8.7)|

</center>

# High level overview

- Uses BLE Mesh network. So, we have two kinds of devices:
   1. Gateway (Acts as BLE Mesh Provisioner - communicates with Cloud over MQTT)
   2. Node (Acts as BLE Mesh slave - communicates with Gateway and other nodes)
- **IMPORTANT NOTE**: Same code base is used to program both Gateway and Node. Only thing to change is a MACRO named **IS_GWY** declared in `main.h` file. It will take care of everything else.
- An open source IR Library called IRremoteESP8366 is used for Transmission and reception of IR Signals. Supports the following brand ACs:
   1. Daikin
   2. LG
   3. Voltas
   4. Samsung
   5. Hitachi
   6. Haier
   7. Carrier
   8. Toshiba
   9. Mitshubishi
- **IMPORTANT NOTE**: Support for the above brands doesn;t mean all models of the above brand AC's would work. The same brand uses different protocols for different models.
- In order to make Universal support (for controlling temperature between 19 and 28 only), we have included a process called Teaching Mode. (More information about this provided below)
- Uses LTE for MQTT communication
- Custom MQTT packets designed according to Project requirements. More details about this in Software documentation. (link provided below)
- Uses IDF-FreeRTOS for implementing Multi-processing application.
- Uses on-board EEPROM for flash storage

## Hardware Details

- [Schematics](https://qmaxltd-my.sharepoint.com/:b:/g/personal/embedded_qmaxsys_com/EcAzDj2xZpRPheBzz8MhQ4MBeOw5IVayl4XTD_MZNdEs2Q?e=pJdu1H)

## Software Documentation and other Helper documents

- Software Documentation: [Link to Software Documentation](https://qmaxltd-my.sharepoint.com/:b:/g/personal/embedded_qmaxsys_com/EVaEBQlBNA1DoCinDPwBE_IBt691awvTQk0ohQl9dxLVjQ?e=Tk5M30)
- Software Flow: [Link To Software Flow PDF](https://qmaxltd-my.sharepoint.com/:b:/g/personal/embedded_qmaxsys_com/ETi3Jls5hMJHh_DprKNjEb8Bi8FhMxQvkYVPa3WIUBaffA?e=LkIWIh)
- MCU : ESP32-S3 [Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- LTE : [Quectel's EC200U Hardware design document](https://forums.quectel.com/uploads/short-url/j0qEXlvPl25PfUDBf4QEkc9AQyx.pdf)
- [AT commands manual](https://forums.quectel.com/uploads/short-url/dV5cK9eteeQmwyGPgfWB351oZde.pdf)
- [TCP/IP command manual](https://forums.quectel.com/uploads/short-url/1loXiVyd118F6AYTRvznNkCbh1L.pdf)
- [MQTT commands manual](https://auroraevernet.ru/upload/iblock/c81/rfhactu9l14ymr9cxt3pebdqxfu39h5v.pdf)

## Software Dependencies

- VS Code IDE
- VS Code ESP-IDF Extension
- ESP-IDF v5.0 or above
- Arduino as ESP component. Check this tutorial : [Adding Arduino as ESP IDF component](https://www.youtube.com/watch?v=hHzGX-K6lmo&pp=ygUfQWRkaW5nIGFyZHVpbm8gYXMgZXNwIGNvbXBvbmVudA%3D%3D)
- IRremoteESP8266 Library [Github Link](https://github.com/crankyoldgit/IRremoteESP8266)
- FreeRTOS --> [Documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/freertos.html)
- BLE Mesh --> [Documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/esp-ble-mesh/ble-mesh-index.html)
- MQTT

## CustomModificiations

1. Alarm.c of ESP-IDF changed.
2. esp_hal gpio logs turned off
3. IR Library changed to include Daikin200 send functions

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

## Steps to perform Teaching Mode

1. Make sure Gateway is registered / Make sure Node is provisioned.
2. **IMPORTANT NOTE (before making the device enter Teaching mode)**: Take the AC remote and bring down the temperature to 19 and power ON the AC. So that, when you press the power button again, it should send the POWER OFF signal . This is very important in-order to successfully register the commands to EEPROM on-board and complete the teaching process. The sequence of IR signals for teaching mode is as follows:
   1. POWER OFF | TEMP 19
   2. POWER ON  | TEMP 19
   3. POWER ON  | TEMP 20
   4. POWER ON  | TEMP 21
   5. POWER ON  | TEMP 22
   6. POWER ON  | TEMP 23
   7. POWER ON  | TEMP 24
   8. POWER ON  | TEMP 25
   9. POWER ON  | TEMP 26
   10. POWER ON | TEMP 27
   11. POWER ON | TEMP 28
3. Double press the side-button on the device. The LED will start blinking BLUE (once every 50ms) to indicate it has entered teaching mode.
4. Press the POWER ON button on the AC remote with remote pointing towards to IR receiver on-board for successful reception and prevent false recording.
5. The device will read the IR signal from the remote and store it in flash. While this process is happening, the light will turn off momentarily to indicate that no other AC remote button must be pressed to let the process go on without disturbance. Also, make sure you are doing this process in a IR disturbance free environment. Most smartphones these days use IR emitters, so it's one thing that I experienced during my development.
6. Once the LED starts blinking BLUE again, it's time to record the next IR signal. The next signal is `POWER ON | TEMP 19`.
7. Similarly, go one-by-one all the way up to `POWER ON | TEMP 28`. If everything was done right, then, when the last IR signal was sent, the LED will change to SOLID GREEN to indicate successful completion of the process.

<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
contributors-shield: <https://img.shields.io/github/contributors/othneildrew/Best-README-Template.svg?style=for-the-badge>
contributors-url <https://github.com/othneildrew/Best-README-Template/graphs/contributors>
forks-shield <https://img.shields.io/github/forks/othneildrew/Best-README-Template.svg?style=for-the-badge>
forks-url <https://github.com/othneildrew/Best-README-Template/network/members>
stars-shield <https://img.shields.io/github/stars/othneildrew/Best-README-Template.svg?style=for-the-badge>
stars-url <https://github.com/othneildrew/Best-README-Template/stargazers>
issues-shield <https://img.shields.io/github/issues/othneildrew/Best-README-Template.svg?style=for-the-badge>
issues-url <http://git.qmaxsys.com/Unimation/c01230-ir-ble-mesh-ac-controller-gwy/issues>
license-shield <https://img.shields.io/github/license/othneildrew/Best-README-Template.svg?style=for-the-badge>
license-url <https://github.com/othneildrew/Best-README-Template/blob/master/LICENSE.txt>
linkedin-shield <https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555>
linkedin-url <https://linkedin.com/in/othneildrew>
product-screenshot images/screenshot.png
Next.js <https://img.shields.io/badge/next.js-000000?style=for-the-badge&logo=nextdotjs&logoColor=white>
Next-url <https://nextjs.org/>
React.js <https://img.shields.io/badge/React-20232A?style=for-the-badge&logo=react&logoColor=61DAFB>
React-url <https://reactjs.org/>
Vue.js <https://img.shields.io/badge/Vue.js-35495E?style=for-the-badge&logo=vuedotjs&logoColor=4FC08D>
Vue-url <https://vuejs.org/>
Angular.io <https://img.shields.io/badge/Angular-DD0031?style=for-the-badge&logo=angular&logoColor=white>
Angular-url <https://angular.io/>
Svelte.dev <https://img.shields.io/badge/Svelte-4A4A55?style=for-the-badge&logo=svelte&logoColor=FF3E00>
Svelte-url <https://svelte.dev/>
Laravel.com <https://img.shields.io/badge/Laravel-FF2D20?style=for-the-badge&logo=laravel&logoColor=white>
Laravel-url <https://laravel.com>
Bootstrap.com <https://img.shields.io/badge/Bootstrap-563D7C?style=for-the-badge&logo=bootstrap&logoColor=white>
Bootstrap-url <https://getbootstrap.com>
JQuery.com <https://img.shields.io/badge/jQuery-0769AD?style=for-the-badge&logo=jquery&logoColor=white>
JQuery-url <https://jquery.com>

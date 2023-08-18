# udp_imu-transmit
 

一个用于采集IMU数据并且进行传输的程序，包含板端的工程，上位机的读取程序，处理端的程序（esp32），使用了自己编写的udp节点通信框架，所有设备在一个局域网下便可自动连接，传输数据。

这个项目是我的ros集合下的一个小项目，验证功能用，可以作为简单的数据读取传输框架修改。

# 文件介绍

│  README.md
│  ros_imu.zip
├─esp_ros_imu_process
│  │  .gitignore
│  │  analysis.py
│  │  platformio.ini 
│  ├─.vscode
│  │      c_cpp_properties.json
│  │      extensions.json
│  │      launch.json
│  │      
│  ├─include
│  │      README
│  │      
│  ├─lib
│  │      README
│  │      
│  ├─src
│  │      comunicate.cpp
│  │      comunicate.h
│  │      config.h
│  │      data_proc.cpp
│  │      data_proc.h
│  │      main.cpp
│  │      
│  └─test
│          README
│          
└─esp_ros_imu
    │  .gitignore
    │  analysis.py
    │  platformio.ini 
    ├─.vscode
    │      c_cpp_properties.json
    │      extensions.json
    │      launch.json
    │      
    ├─include
    │      README
    │      
    ├─lib
    │      README
    │      
    ├─src
    │      comunicate.cpp
    │      comunicate.h
    │      config.h
    │      data_proc.cpp
    │      data_proc.h
    │      JY901.cpp
    │      JY901.h
    │      main.cpp
    │      ws2812.h
    │      ws2812B.cpp
    │      
    └─test
            README
            

### esp_ros_imu: 

运行在imu 板上的工程，imu板的三款设计都在，ros_imu.zip 的压缩包中，立创eda专业版工程。
其中有个analysis.py文件是在另一个工程没有运行时，和电脑通信的程序，使用自写的节点通信框架，同一个局域网下可以自己发现设备并且展现数据
src文件是项目代码所在，数据的读写逻辑在data_proc.cpp中。

### esp_ros_imu_process: 

运行在另一块esp32板子上的工程，可以进行数据接收，处理，转发

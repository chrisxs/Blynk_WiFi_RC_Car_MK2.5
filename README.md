#  **以往版本的选型历史记录:** 

- 原型机-NodeMCU v2(CP2102)+L293D扩展板 

- MK.1.1-NodeMCU v2(CP2102)+L293D扩展板  

- MK.2.1-NodeMCU v3(CH340)+L293D驱动模块

- MK.2.2-NodeMCU v2(CP2102)+L293D扩展板


<center>MK1为遥感版,MK2系列为按钮版</center>


# **ArduinoIDE:**
- IDE版本:1.8.10
- ESP8266库:2.4.2
- Blynk版本:0.6.1
- ArduinoJson:5.13.3
- WiFiManager:0.14.4

# **PlatformIO编译通过的库版本:**
- Blynk:0.6.1   
- ArduinoJson:5.13.4 
- WifiManager:0.14   
- ESP8266:2.0.4、2.1.1、2.2.0  

## **2020-01-13:**
在PlatformIO 2.3.2板库下编译会出错,2.2.3编译成功


----
# **更新Note:**

## **2020-02-21**
### **MK2.5**
1.  优化代码
2.  修改`platformio.ini`文件,让IDE自动下载好相关依赖库,先用命令行运行:

-   下载python工具

    `sudo pip install -U pip setuptools`

    `sudo pip install -U platformio`

-   编译命令

    `platformio run`

- 其他命令
  - `platformio run` - 编译
  - `platformio run -e generic -t upload` - 编译并上传
  - `platformio run -t clean`- 移除已编译文件

**IDE会自动安装,如果是第一次运行,会自动下载相关依赖库(包含对应的版本)**
  


## **2020-01-26:**

### **MK2.5**

#### 引入新功能


  1.加入OLED驱动,显示启动/重置/网络/电压等相关信息;
  
  2.加入调速功能,使用滑动条设置PWM速度;

---

## 2020-01-17:
### **MK.I:**
#### 引脚变动

 1. 项目-取消舵机、远近光灯的功能和所占引脚、资源，控制摄像头拍摄直接使用带云台和夜视功能的成品，照明功能用红外夜视代替;
 2. 加入复位功能按钮（D5），按下按钮不放，点“Reset”，进入WIFI配网；
 
 #### 小变动

 1. 精简车身,轻量化,链接线路简化,代码简化,电源使用改用8.4V锂电驱动;
 2. 保留摇杆控制(电机接线与MK2版本相反);
 3. 保留电量电压采集(代码保留);
 
### 其他更新（MK.1/MK2共有）

 1. 加入OTA更新功能；
 2. 加入实时显示SSID、IP、MAC、RSSI信息功能；



---

## **2020-01-17:**
### **MK.II:**
#### 变动
 1. 修改电机驱动代码,使内网版APP控制界面与BLYNK物联网平台保持一致;
 2. 统一反馈标准,如重置引脚、配网LED提示、平台接入提示
---




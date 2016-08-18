# **ProbeDroid**  

ProbeDroid is a ***dynamic Java code instrumentation kit for Android application***. It provides APIs for users to craft their own instrumentation tools. Thus they can trace, profile, or change the runtime behavior of an interested application. Essentially, ***Java method*** is the ***basic instrumentation unit***. To manipulate the interested methods, users should override the template instrumentation gadgets and register them to hook the interested methods. During runtime, when those methods are invoked, the control flow is diverted to the gadgets. And it is the timing to manipulate the boxed method input arguments and return value. In the current stage, ProbeDroid targets on Android 5.0 and above. To build ProbeDroid kit, users just pull the package from GitHub and follow the build commands. Android source tree is not required.  

## **Feature**  
+  Programmable instrumentation
   +  Code your own instrument tools with ***Java practice***
   +  Flexible APIs for you to
      +  Hook interested library or app defined methods
      +  Customize instrument gadgets for different analysis purposes
      +  Modify method in/output to hack app at runtime
+  Succinct deployment
   +  Only ProbeDroid engine and instrument tools are required
   +  No need to customize Android framework


## **[Design Memo]**


## **Limitation**  
+  Cannot instrument ***native*** methods now (under development)  
+  Currently only supporting ***Android 5.0*** and the devices based on ***Intel x86*** and ***ARM eabi v7a*** 

#### ProbeDroid is still under construction. More features will be presented in the near feature. 

<img src="https://github.com/ZSShen/ProbeDroid/blob/master/res/ProbeDroidOverview.png"/width="750px">


## **Installation**
Please refer to [Source Building Wiki]

## **Usage**
Please refer to [Play and Hack Wiki]

## **Demo**

#### **Instrument GoogleMaps**
**Click the picture to view the demo vedio**  

| [![GoogleMaps](http://img.youtube.com/vi/6_kg-229yz4/hqdefault.jpg)](https://www.youtube.com/watch?v=6_kg-229yz4&nohtml5=False) |
|---|
| A simple instrumentation tool which ***tracks the strings converted from StringBuilder and StringBuffer object***. By taking some forensics towards the converted strings, you would notice that GoogleMaps applies Java reflection for some network authentication. Also, It will dynamically generate some C/C++ code and compile it for map rendering. |


#### **Instrument KKTix**
**Click the picture to view the demo vedio**  

| [![KKTix](http://img.youtube.com/vi/KV8gRs0xWQ8/hqdefault.jpg)](https://www.youtube.com/watch?v=KV8gRs0xWQ8) |
|---|
| A simple instrumentation tool which ***tracks the started Activities and Services***. By taking some forensics towards the tracked components, you would notice that KKTix applies several kinds of Activies to render the ticket booking pages. Also, it starts a  Service for background computation. |

## **License**
Except for the following source code:  
+ `android/art/runtime/`, `common/log.*`, `common/stringprintf.*`, `common/utf.*`, and `common/macros.h` subtrees belong to [AOSP], which are licensed under ***Apache v2.0***.  
+ `common/libffi/` subtree belongs to [libffi], which is licensed under ***MIT***.   

All the source code are licensed under ***MIT***. See ***COPYING*** for details.  


## **Contact**
Please contact me via the mail ***andy.zsshen@gmail.com***.  
Note that the kit is still under construction.  Contribution and bug report is desired.  

[Design Memo]:http://www.slideshare.net/ZongShenShen/probedroid-crafting-your-own-dynamic-instrument-tool-on-android-for-app-behavior-exploration
[Source Building Wiki]:https://github.com/ZSShen/ProbeDroid/wiki/Road-Map
[Play and Hack Wiki]:https://github.com/ZSShen/ProbeDroid/wiki/Road-Map

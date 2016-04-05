# **ProbeDroid**  

A ***process level dynamic binary instrumentation kit***  for analysts to manipulate ***java methods*** on the fly. In short, ProbeDroid provides a ***Java library*** for analysts to craft their own instrumentation packages. Analysts can register gadgets to monitor the interested Java methods. Furthermore, by modifying the method arguments and return value, the application behavior can be dynamically altered as they wish.  

## **Features**  
+  Process level
+  Dynamic Java method instrumentation
+  Java library as integration interface
+  Currently supporting Android(L) 5.0 and Intel x86 ISA

<img src="https://github.com/ZSShen/ProbeDroid/blob/master/res/ProbeDroidOverview.png"/width="750px">

## **How to Compile ProbeDroid Runtime**  

### **1. Prerequisite**
+  [Android NDK]  
+  [Apache Ant]  

Note that,  the Android ***API level*** should be ***21***.

### **2. Terminology**
The ProbeDroid Runtime is composed of the ***Launcher*** and the ***Engine***:  
+  ***Launcher*** - Used to inject the engine and the custom instrumentation package into the target process.  
+  ***Engine*** - Used to marshal the control flow between Android Runtime and the instrumentation package.  

The frequently used path name identifiers:  
+  ***`PATH_IN_HOST`*** - The absolute path storing ProbeDroid Runtime source in your host machine.  
+  ***`PATH_IN_DEVICE`*** - The working directory in your experiment device. `/data/local/tmp` is recommended.  

### **3. Build Launcher**
1.  Switch to `PATH_IN_HOST/launcher/jni`, and type:  
    ```
    $ ndk-build
    ```  

    The launcher executable should reside in `PATH_IN_HOST/launcher/libs/x86/launcher`.  

### **4. Build Engine**
1.  Switch to `PATH_IN_HOST/engine/jni`, and type:  
    ```
    $ ndk-build
    ```  

    The native library should reside in `PATH_IN_HOST/engine/libs/x86/libProbeDroid.so`.  

2.  Switch to `PATH_IN_HOST/engine`, and type:  
    ```
    $ ant compile
    $ ant build-jar
    ```  

    The java library jar should reside in `PATH_IN_HOST/engine/ProbeDroid.jar`.  


## **How to Develop Instrumentation Package**



## **How to Launch Instrumentation**



## **License**
Except for the following source code:  
+ `android/art/runtime/`, `common/log.*`, `common/macros.h`, `common/stringprintf.*`, and `common/utf.*` subtrees are licensed under ***Apache v2.0***.  
+ `common/libffi/` subtree is licensed under ***GNU GLP v2.0***.   

All the source code are licensed under ***MIT***. See ***COPYING*** for details.  

## **Contact**
Please contact me via the mail ***andy.zsshen@gmail.com***.  
Note that the kit is still under construction.  Contribution and bug report is desired.  

[Android SDK]:https://developer.android.com/intl/sdk/index.html
[Android NDK]:http://developer.android.com/intl/tools/sdk/ndk/index.html
[Apache Ant]:http://ant.apache.org/

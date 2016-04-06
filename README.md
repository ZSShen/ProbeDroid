# **ProbeDroid**  

A ***process level dynamic binary instrumentation kit***  for analysts to manipulate ***java methods*** on the fly. In short, ProbeDroid provides a ***Java library*** for analysts to craft their own instrumentation packages. Analysts can register gadgets to monitor the interested Java methods. Furthermore, by modifying the method arguments and return value, the application behavior can be dynamically altered as they wish.  

## **Feature**  
+  Process level dynamic Java method instrumentation
+  Java library as integration interface


## **Limitation**
+  Currently supporting Android(L) 5.0 and Intel x86 ISA
+  Only tested in Android virtual device 


<img src="https://github.com/ZSShen/ProbeDroid/blob/master/res/ProbeDroidOverview.png"/width="750px">


## **Required Build and Deployment Kit**
+  [Android SDK]  
+  [Android NDK]
+  [Apache Ant]


## **How to Compile ProbeDroid Runtime**  
We need [Android NDK] and [Apache Ant] to build ProbeDroid Runtime.

#### **1. Terminology**
The ProbeDroid Runtime is composed of the ***Launcher*** and the ***Engine***:  
+  ***Launcher*** - Used to inject the engine and the custom instrumentation package into the target process.  
+  ***Engine*** - Used to marshal the control flow between Android Runtime and the instrumentation package.  

The frequently used path name identifiers:  
+  ***`PATH_IN_HOST`*** - The absolute path storing ProbeDroid Runtime source in your host machine.  
+  ***`PATH_IN_DEVICE`*** - The working directory in your experiment device. `/data/local/tmp` is recommended.  

#### **2. Build Launcher**
1.  Switch to `PATH_IN_HOST/launcher/jni`, and type:  
    ```
    $ ndk-build
    ```  

    The launcher executable should reside in `PATH_IN_HOST/launcher/libs/x86/launcher`.  

#### **3. Build Engine**
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
We recommend you to apply [Android Studio] for development.  

1.  Create an Android Studio project.  

2.  Import the library jar `ProbeDroid.jar` into the project.  

3.  Develop your own instrumentation tool with the following resource:  
    1.  Refer to [JavaDoc] for the usage of ProbeDroid API.  
    2.  Some [Sample Tools] are provided.  

4. Compile the project as android APK.  

For the detailed development information, please refer to the wiki page [How to Develop Instrumentation Package].


## **How to Launch Instrumentation**



## **License**
Except for the following source code:  
+ `android/art/runtime/`, `common/log.*`, `common/stringprintf.*`, `common/utf.*`, and `common/macros.h` subtrees belong to [AOSP], which are licensed under ***Apache v2.0***.  
+ `common/libffi/` subtree belongs to [libffi], which is licensed under ***GNU GLP v2.0***.   

All the source code are licensed under ***MIT***. See ***COPYING*** for details.  


## **Contact**
Please contact me via the mail ***andy.zsshen@gmail.com***.  
Note that the kit is still under construction.  Contribution and bug report is desired.  

[Android SDK]:http://developer.android.com/sdk/index.html
[Android NDK]:http://developer.android.com/ndk/index.html
[Apache Ant]:http://ant.apache.org/
[AOSP]:https://source.android.com/
[libffi]:https://sourceware.org/libffi/
[Android Studio]:http://developer.android.com/sdk/index.html

[How to Develop Instrumentation Package]:https://github.com/ZSShen/ProbeDroid/wiki/How-to-Develop-Instrumentation-Package
[JavaDoc]:http://zsshen.github.io/ProbeDroid/doc/index.html
[Sample Tools]:https://github.com/ZSShen/ProbeDroid/tree/master/tools

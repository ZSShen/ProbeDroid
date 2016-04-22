# **ProbeDroid**  

ProbeDroid is a ***dynamic Java code instrumentation kit for Android application***. It exports APIs for developers to craft their own analysis tools. Thus they can trace, profile, or change the behavior of a designated application when it is launched. Essentially, a ***Java method*** is the basic instrumentation unit. Developers can designate the interested Java methods with the provided APIs. Also, the template callbacks are prepared. Developers can override and register the callbacks to fit the need for the interested target . Therefore, when the target is invoked, or when it is finished, the control is passed to the callbacks. And it is the timing to manipulate the boxed input arguments and return value. Currently, ProbeDroid targets on Android 5.0 and above. To build the ProbeDroid kit, developers just pull the package from GitHub and build it by following the commands listed in the document. Android source tree is not required.   


## **Feature**  
+  Dynamic Java method instrumentation kit for Android app  
   +  Java library as integration interface for developers  
   +  Templates for developer to customize the analysis procedure for the interest methods  
   +  Modifiable captured method input arguments and return value  
+  Direct build procedure ***without Android source dependency***  
   +  Pulling the GitHub source and following the document  
+  Succinct deployment ***without customized ROM or system apps***
   +  Only ProbeDroid engine and developers' instrumentation package are required

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

+  Launcher - Used to inject the engine and the custom instrumentation package into the target process.  
+  Engine - Used to marshal the control flow between Android Runtime and the instrumentation package.  

The frequently used path name identifiers:  

+  `PATH_IN_HOST` - The absolute path storing ProbeDroid Runtime source in your host machine.  

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

1.  Create an Android Studio project.  The ***API level*** should be ***21***.

2.  Import the library jar `ProbeDroid.jar` into the project.  

3.  Develop your own instrumentation tool with the following resource:  
    1.  Refer to [JavaDoc] for the usage of ProbeDroid API.  
    2.  Some [Sample Tools] are provided.  

4. Compile the project as android APK.  

For the detailed development information, please refer to the wiki page [How to Develop Instrumentation Package].


## **How to Start Instrumentation**
We need [Android SDK] to create the virtual device.

#### **1. Terminology**
The frequently used path name identifiers:  

+  `PATH_IN_HOST` - The absolute path storing ProbeDroid Runtime source in your host machine.
+  `PATH_IN_DEVICE` - The working directory in your experiment device. `/data/local/tmp` is recommended.
+  `PATH_YOUR_PACKAGE` - The path storing your instrumentation package.

#### **2. Create Virtual Device**
1.  Create a virtual device with ***Intel x86 ISA*** and ***API level 21***.  

2.  Boot up the virtual device.

3.  Turn off the ***SEAndroid*** mandatory access control:
    ```
    $ su 0 setenforce 0
    ``` 

#### **3. Deploy ProbeDroid Runtime**
1.  Move the ProbeDroid launcher to the experiment device:  
    ```
    $ adb push PATH_IN_HOST/launcher/libs/x86/launcher PATH_IN_DEVICE/
    $ chmod a+x PATH_IN_DEVICE/launcher
    ```

2.  Move the ProbeDroid engine to the experiment device:  
    ```
    $ adb push PATH_IN_HOST/engine/libs/x86/libProbeDroid.so  PATH_IN_DEVICE/
    $ chmod a+x PATH_IN_DEVICE/libProbeDroid.so
    ```

#### **4. Deploy Instrumentation Package**
1.  Move your instrumentation package to the experiment device:  
    ```
    $ adb push PATH_YOUR_PACKAGE PATH_IN_DEVICE
    ```

#### **5. Start Instrumentation**
1.  Run the ProbeDroid launcher.  
    ```
        Usage: ./launcher
            --app    [-a] APPNAME    (The package name (or keyword) of the target app)
            --lib    [-l] LIBPATH    (The *absolute path name* of libProbeDroid.so)
            --module [-m] MODULEPATH (The *absolute path name* of your instrumentation package)
            --class  [-c] CLASSNAME  (The *fully qualified main class name* of your instrumentation package)
    ```  

2.  Monitor the message spewed by logcat daemon.  

For the detailed deployment information, please refer to the wiki page [How to Start Instrumentation].

## **Demo**

#### **Instrument GoogleMaps**
**Click the picture to view the demo vedio**  

| [![GoogleMaps](http://img.youtube.com/vi/6_kg-229yz4/hqdefault.jpg)](https://www.youtube.com/watch?v=6_kg-229yz4&nohtml5=False) |
|---|
| A simple instrumentation tool which ***tracks the strings converted from StringBuilder object***. By taking some forensics towards the converted strings, you would notice that GoogleMaps applies Java reflection to accomplish some network authentication. Also, It will dynamically generate some C/C++ code and compile it for map rendering. |


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

[Android SDK]:http://developer.android.com/sdk/index.html
[Android NDK]:http://developer.android.com/ndk/index.html
[Apache Ant]:http://ant.apache.org/
[AOSP]:https://source.android.com/
[libffi]:https://sourceware.org/libffi/
[Android Studio]:http://developer.android.com/sdk/index.html

[How to Develop Instrumentation Package]:https://github.com/ZSShen/ProbeDroid/wiki/How-to-Develop-Instrumentation-Package
[How to Start Instrumentation]:https://github.com/ZSShen/ProbeDroid/wiki/How-to-Start-Instrumentation
[JavaDoc]:http://zsshen.github.io/ProbeDroid/doc/index.html
[Sample Tools]:https://github.com/ZSShen/ProbeDroid/tree/master/tools

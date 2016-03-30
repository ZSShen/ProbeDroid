# **ProbeDroid**  

## **Prologue**  
A ***dynamic binary instrumentation toolkit*** for analysts to manipulate application on the fly. Currently, it targets on ***Android(L) 5.0*** and ***Intel-x86 ISA***, the  support for the newest Android and other machine architectures would be provided in the near future. In short, ProbeDroid offers a ***Java library*** for analysts to craft their own instrumentation packages. Analysts can register hooks to monitor the interested Java methods. Furthermore, by modifying the method arguments and return value, the application behavior can be dynamically altered as they wish.  However, ProbeDroid ***has only been tested in AVD***. For the real phone deployment, the stability is not guaranteed.  Specifically, you have to root the phone.

#### **ProbeDroid API**
[Link to JavaDoc] (http://zsshen.github.io/ProbeDroid/doc/index.html)



## **Prerequisite**  
The fundamental development environment:  
- [Android SDK] - To build the instrumentation package and to create the AVD.  
- [Android NDK] - To build the ProbeDroid core.  
- [Apache Ant] - To build the ProbeDroid core.  

Note that,  the Android API level should be at least 21.  

## **Installation**  
The toolkit is composed of the ***launcher*** and the ***core libraries***, and we illustrate the installation steps respectively.  
Some terms should be defined first:  
- `PATH_IN_HOST` - The absolute path storing ProbeDroid source in your host machine.  
- `PATH_IN_DEVICE` - The working directory in your experiment device. `/data/local/tmp` is recommended.  
 
#### **For Launcher**  
1.  Switch to `PATH_IN_HOST/inject/jni`, and type:  
    ```
    $ ndk-build
    ``` 
    The executable should reside in `PATH_IN_HOST/inject/libs/x86/inject`.  

2.  Push the executable into the experiment device:  
    ```
    $ adb push PATH_IN_HOST/inject/libs/x86/inject PATH_IN_DEVICE/
    ```

3.  And in the device, change the access permission:  
    ```
    $ chmod a+x PATH_IN_DEVICE/inject
    ```

---

#### **For Core Libraries**  
The core libraries can be further divided into two parts: the native library and the Java jar.  

##### **We illustrate how to build the native library first**  
1.  Switch to `PATH_IN_HOST/payload/jni`, and type:  
    ```
    $ ndk-build
    ``` 
    The library should reside in `PATH_IN_HOST/payload/libs/x86/libProbeDroid.so`.  

2.  Push the library into the experiment device:  
    ```
    $ adb push PATH_IN_HOST/payload/libs/x86/libProbeDroid.so  PATH_IN_DEVICE/
    ```

3.  And in the device, change the access permission:  
    ```
    $ chmod a+x PATH_TO_WORK/libProbeDroid.so
    ```

##### **Now we illustrate how to build the Java jar**  
1.  Switch to `PATH_IN_HOST/payload/`, and type:  
    ```
    $ ant compile
    $ ant build-jar
    ``` 
    The jar file should reside in `PATH_IN_HOST/payload/ProbeDroid.jar`.  

## **Usage**  
We first illustrate how to apply ProbeDroid API to build your own instrumentation package. Then the approach to inject the package into the interested app will be described.  

#### **Development**
For efficiency, we recommend you to apply ***Android Studio*** for development.  

1.  Import ***ProbeDroid.jar*** into your project.
Check this post http://stackoverflow.com/questions/16608135/android-studio-add-jar-as-library if you are not familiar with importing external jar into Android Studio project.  

2.  Refer to ProbeDroid API document to craft your own instrumentation package. In the directory `PATH_IN_HOST/tools`, some sample tools are already provided. It would be helpful to read the code first.  

3.  Compile the Android Studio project as android APK. However, Android Studio now has some issues to compile the project importing external java jar. To fix the issue, add the following configuration into your ***build.gradle***. 
    ```
    compileOptions {
        sourceCompatibility JavaVersion.VERSION_1_7
        targetCompatibility JavaVersion.VERSION_1_7
    }
    ```
    You can also refer to the sample tools in `PATH_IN_HOST/tools` to check the complete build configuration.  

---

#### **Deployment**
1.  Since ***current Android enforces SELinux mandatory access control***. To force an arbitrary application load and execute ProbeDroid, we must turn off the access control. Note that ***we can only switch SELinux to advisory mode (Warning but Non-Blocking) instead of complete shutdown mode***.  
For this, in the experiment device, just type:  
    ```
    $ su 0 setenforce 0
    ```

2.  For more concrete specification, we use ***GoogleMaps*** as the sample to demonstrate how to start instrumentation.
    1.  Push the instrumentation APK to the working directory of your experiment device.  
        ```
        $ adb push PATH_OF_YOUR_INSTRUMENT_APK  PATH_IN_DEVICE
        ```

    2.  Ensure that GoogleMaps is not active. Just type:  
        ```
        $ ps
        ```
        to resolve the process id by searching for the GoogleMaps process name ***com.google.android.apps.maps***. If it is active, apply:  
        ```
        $ kill PID
        ```
        to terminate the process.

    3.  Resolve the ***zygote*** process id with `ps` command again.  

    4.  Run the instrumentation launcher by specifying the following command arguments:  
        ```
        Usage: ./inject
            --zygote [-z] PID        (The zygote process id)
            --app    [-a] APPNAME    (The keyword of target app package name)
            --lib    [-l] LIBPATH    (The path name of ProbeDroid core library)
            --module [-m] MODULEPATH (The path name of your analysis package)
            --class  [-c] CLASSNAME  (The fully qualified main class name of your analysis package)
        ```
        ```
        Example:
        $ ./inject -z 934 -a maps -l /data/local/tmp/libProbeDroid.so -m /data/local/tmp/Instument.apk org.zsshen.instument.Main
        ```

    5. Start GoogleMaps by using Android `am` command or touching the GoogleMaps icon shown by Android Launcher.  

    6. If the instrumentation started successfully, you will see the similar message like this:  
        <img src="https://github.com/ZSShen/ProbeDroid/blob/master/res/TaskStarted.png" width="450px"/>  

        Then you can check the ***logcat*** debug message shown in Android Studio to follow the process.  
        <img src="https://github.com/ZSShen/ProbeDroid/blob/master/res/GadgetDeployment.png" />  

    7. If you plan to terminate the instrumentation, type:  
        ```
        $ kill -SIGTERM APP_PID
        ```  
        After finalization, you can acquire the profiled data if any.  

3.  You can also refer to the following demo videos.
    1.  [Simple tool to instrument GoogleMaps](https://www.youtube.com/watch?v=6_kg-229yz4)
    2.  [Simple tool to instrument KKTix](https://www.youtube.com/watch?v=KV8gRs0xWQ8)

## **Epilogue**
If you have any questions, please contact me via the mail: andy.zsshen@gmail.com  
Please note that the toolkit is still under construction.  Contribution and bug report is welcome.  

[Android SDK]:https://developer.android.com/intl/sdk/index.html
[Android NDK]:http://developer.android.com/intl/tools/sdk/ndk/index.html
[Apache Ant]:http://ant.apache.org/

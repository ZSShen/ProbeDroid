##ProbeDroid

##Prologue 
A ***dynamic binary instrumentation toolkit*** for analysts to manipulate application on the fly. Currently, it targets on ***Android(L) 5.0*** and ***Intel-x86 ISA***, the  support for the newest Android and other machine architectures would be provided in the near future. In short, ProbeDroid offers a ***Java SDK*** for analysts to craft their own instrument packages. Analysts can register hooks to monitor the interested Java methods. Furthermore, by modifying the method arguments and return value, the application behavior can be dynamically altered as they wish.  However, ProbeDroid ***has only been tested in AVD***. For the real phone deployment, the stability is not guaranteed.  

##Prerequisite 
The fundamental development environment:  
- [Android SDK] - To build the instrumentation package and to create the AVD.
- [Android NDK] - To build the ProbeDroid core.  
- [Apache Ant] - To build the ProbeDroid core.
Note that,  the API level should be at least 21.

##Installation
The toolkit is composed of the ***launcher*** and the ***core libraries***, and we illustrate the installation steps respectively.  
Some terms should be defined first:  
- `PATH_TO_HOST` - The absolute path storing ProbeDroid source in your host machine.  
- `PATH_TO_DEVICE` - The working directory in your analysis device.  
 
####*For Launcher* 
Firstly, switch to `PATH_TO_HOST/inject/jni`:  
```sh
$ ndk-build
``` 
The executable will reside in `PATH_TO_HOST/inject/libs/x86/inject`  

Secondly, push the executable into the target device:  
```sh
$ adb push PATH_TO_HOST/inject/libs/x86/inject PATH_TO_DEVICE/
```
And in the device, change the access permission:  
```sh
$ chmod a+x PATH_TO_DEVICE/inject
```

####*For Core Libraries*  
The core libraries can be further divided into two parts: the native library and the Java jar.  

#####*We illustrate how to build the native library first*
Firstly, switch to `PATH_TO_HOST/payload/jni`:  
```sh
$ ndk-build
``` 
The library will reside in `PATH_TO_HOST/payload/libs/x86/libProbeDroid.so`  

Secondly, push the library into the target device:
```sh
$ adb push PATH_TO_HOST/payload/libs/x86/libProbeDroid.so  PATH_TO_DEVICE/
```
And in the device, change the access permission:
```sh
$ chmod a+x PATH_TO_WORK/libProbeDroid.so
```

#####*Now we illustrate how to build the Java jar*
Firstly, switch to `PATH_TO_HOST/payload/`:  
```sh
$ ant compile
$ ant build-jar
``` 
The jar file will reside in `PATH_TO_HOST/payload/ProbeDroid.jar`  


## Usage


##Epilogue
If you have any questions, please contact me via the mail: andy.zsshen@gmail.com  
Please note that the toolkit is still under construction.  Contribution and bug report is welcome.  

[Android SDK]:https://developer.android.com/intl/sdk/index.html
[Android NDK]:http://developer.android.com/intl/tools/sdk/ndk/index.html
[Apache Ant]:http://ant.apache.org/

##DroidProf

###Prologue 
A dynamic binary instrumentation toolkit for application profiling targeting on Android(L) 5.0 and above. Hopefully, it will fly. But it is now under the POC construction. In more detail, ***shared library injection*** is the fundamental technique to support this toolkit. To profile the given application, a component of this toolkit called **injector** will inject a **hooking library** into the target. The library will hook several functions which interact with the class fields and methods of an application, and thus accomplish the goal for behavior profiling. ***In the current stage, I just finish the injector, and the hooking library is now under way***.  Besides, a primary restriction is that the toolkit is now focusing on Intel x86 architecture.

###Prerequisite 
We need the fundamental Android toolchain:  
- [Android SDK] - To create the AVD targeting on Intel x86 for experiment.  
- [Android NDK] - To build the profiling toolkit.  
Note that,  the API level should be at least 21.

###Installation
The toolkit is composed of injector and hooking library, and we illustrate the installation steps respectively.  
Suppose that:  
- The absolute path storing DroidProf in your host machine is `PATH_TO_DROIDPROF` .  
- The working directory in your AVD is `PATH_TO_WORK`.  
 
####For injector 
Firstly, we switch to `PATH_TO_DROIDPROF/inject/jni` to build the executable:  
```sh
$ ndk-build
``` 
The executable will reside in `PATH_TO_DROID_PROF/inject/libs/x86/inject`  

Secondly, we push the executable into AVD:
```sh
$ adb push PATH_TO_DROID_PROF/inject/libs/x86/inject PATH_TO_WORK/
```
And change its access permission:
```sh
$ chmod a+x PATH_TO_WORK/inject
```

####For hooking library  
Firstly, we switch to `PATH_TO_DROIDPROF/payload/jni` to build the library:  
```sh
$ ndk-build
``` 
The library will reside in `PATH_TO_DROID_PROF/payload/libs/x86/libhook.so`  

Secondly, we push the library into AVD:
```sh
$ adb push PATH_TO_DROID_PROF/payload/libs/x86/libhook.so PATH_TO_WORK/
```
And change its access permission:
```sh
$ chmod a+x PATH_TO_WORK/libhook.so
```



[Android SDK]:https://developer.android.com/sdk/index.html#Other
[Android NDK]:http://developer.android.com/ndk/downloads/index.html

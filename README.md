##DroidProf

##Prologue 
A dynamic binary instrumentation toolkit for application profiling targeting on Android(L) 5.0 and above. Hopefully, it will fly. But it is now under the POC construction. In more detail, ***shared library injection*** is the fundamental technique to support this toolkit. To profile the given application, a component of this toolkit called **injector** will inject a **hooking library** into the target. The library will hook several functions which interact with the Java class fields and methods of an application, and thus accomplish the goal for behavior profiling. ***In the current stage, I just finish the injector, and the hooking library is now under way***.  Besides, a primary restriction is that the toolkit is now focusing on Intel x86 architecture.

##Prerequisite 
We need the fundamental Android toolchain:  
- [Android SDK] - To create the AVD targeting on Intel x86 for experiment.  
- [Android NDK] - To build the profiling toolkit.  
Note that,  the API level should be at least 21.

##Installation
The toolkit is composed of injector and hooking library, and we illustrate the installation steps respectively.  
Suppose that:  
- The absolute path storing DroidProf in your host machine is `PATH_TO_DROIDPROF` .  
- The working directory in your AVD is `PATH_TO_WORK`.  
 
####*For Injector* 
Firstly, we switch to `PATH_TO_DROIDPROF/inject/jni` to build the executable:  
```sh
$ ndk-build
``` 
The executable will reside in `PATH_TO_DROID_PROF/inject/libs/x86/inject`  

Secondly, we push the executable into AVD:
```sh
$ adb push PATH_TO_DROID_PROF/inject/libs/x86/inject PATH_TO_WORK/
```
And in the AVD, change the access permission of injector:
```sh
$ chmod a+x PATH_TO_WORK/inject
```

####*For Hooking Library*  
Firstly, we switch to `PATH_TO_DROIDPROF/payload/jni` to build the library:  
```sh
$ ndk-build
``` 
The library will reside in `PATH_TO_DROID_PROF/payload/libs/x86/libhook.so`  

Secondly, we push the library into AVD:
```sh
$ adb push PATH_TO_DROID_PROF/payload/libs/x86/libhook.so PATH_TO_WORK/
```
And in the AVD, change the access permission of hooking library:
```sh
$ chmod a+x PATH_TO_WORK/libhook.so
```

##Usage  
#### *Preprocess before Experiment*  
Firstly, ***current Android enforces SELinux mandatory access control***. To force an arbitrary application load and execute our hooking library, we must temporarily turn off the access control. Note that ***we can only switch SELinux to advisory mode (Warning but Non-Blocking) instead of complete shutdown mode***.  
For this, in your AVD, just type:  
```sh
$ su 0 setenforce 0
```
Secondly, some applications may install background services which will be  automatically triggered by certain Intent messages. ***To ensure the clean experiment and correct library injection on your target application, you have to shutdown its relevant service processes before running toolkit***.  
Suppose your target app is google maps, then you can try the following steps:  
```
$ ps
```
<img src="https://raw.githubusercontent.com/ZSShen/DroidProf/master/res/picture/PS Google Maps.png"/>  
Find the PID of google maps and kill it.  
```sh
$ kill PID
```

####*Experiment Steps*
Essentially, Android applications are forked and initialized by **zygote**. To profile the complete runtime behavior of the target application, the hooking library should be injected into application process before it starts execution. Therefore, we have to rely on zygote for process fork event so that we can:  
- Pause the newly forked application process  
- Inject the hooking library  
- Hook several critical points to monitor Java class field and method  
- Restart the application process  

Seems like a laborious manual effort? Do not worry! The injector already handles the effort you. All you have to do is providing the relevant command line arguments.  
The usage of the injector is listed below:  
```
./inject -z ZYGOTE_PID -a APP_PKG_KEYWORD -l /PATH_TO_WORK/libhook.so
```  

You can acquire the zygote PID by `ps` command.  
<img src="https://raw.githubusercontent.com/ZSShen/DroidProf/master/res/picture/PS Zygote.png"/>  
And for the Google maps example, the complete command should like this:  
```sh
$ ./inject -z 936 -a maps -l /PATH_TO_WORK/libhook.so
```  

If the command finishes successfully, you should see the result like:  
<img src="https://raw.githubusercontent.com/ZSShen/DroidProf/master/res/picture/Command Execution.png"/>  

You can verify the successful library injection by checking the process memory layout:  
```sh
$ cat /proc/PID/maps
```
<img src="https://raw.githubusercontent.com/ZSShen/DroidProf/master/res/picture/Proc Map.png"/>  

Also, the system log can help you extract the debug message printed by the constructor of the library:  
```sh
$ logcat -v time -f /dev/kmsg | cat /proc/kmsg
```  
<img src="https://raw.githubusercontent.com/ZSShen/DroidProf/master/res/picture/Library Launch.png"/>  


###Epilogue
If you have any questions, please contact me via the mail: andy.zsshen@gmail.com  
But please note that the hooking library is now under construction.  
I will update it soon. :)  

[Android SDK]:https://developer.android.com/sdk/index.html#Other
[Android NDK]:http://developer.android.com/ndk/downloads/index.html

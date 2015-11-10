##DroidProf

###Prologue
---
A dynamic binary instrumentation toolkit for application profiling targeting on Android(L) 5.0 and above. Hopefully, it will fly. But it is now under the POC construction. In more detail, ***shared library injection*** is the fundamental technique to support this toolkit. To profile the given application, a component of this toolkit called **injector** will inject a **hooking library** into the target. The library will hook several functions which interact with the class fields and methods of an application, and thus accomplish the goal for behavior profiling. ***In the current stage, I just finish the injector, and the hooking library is now under way***.  Besides, a primary restriction is that the toolkit is now focusing on Intel x86 architecture.

###Prerequisite
---
***This chapter illustrates some basic requirements to run the toolkit***  
We need the fundamental Android toolchain:  
- [Android SDK] - To create the AVD targeting on Intel x86 for experiment.  
- [Android NDK] - To build the profiling toolkit.  
Note that, API level should be at least 21.


[Android SDK]:https://developer.android.com/sdk/index.html#Other
[Android NDK]:http://developer.android.com/ndk/downloads/index.html

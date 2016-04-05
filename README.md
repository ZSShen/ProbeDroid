# **ProbeDroid**  

A ***process level dynamic binary instrumentation kit***  for analysts to manipulate ***java methods*** on the fly. In short, ProbeDroid provides a ***Java library*** for analysts to craft their own instrumentation packages. Analysts can register gadgets to monitor the interested Java methods. Furthermore, by modifying the method arguments and return value, the application behavior can be dynamically altered as they wish.  

## **Features**  
+ Dynamic Java method instrumentation
+ Java library as integration interface
+ Currently supporting Android(L) 5.0 and Intel x86 ISA

<img src="https://github.com/ZSShen/ProbeDroid/blob/master/res/ProbeDroidOverview.png"/width="750px">

## **How to Compile the Engine**  



## **How to Develop the Analysis Package**



## **How to Launch the Instrumentation**



## **License**
Except for the following source code:  
+ `android/art/runtime/`, `common/log.*`, `common/macros.h`, `common/stringprintf.*`, and `common/utf.*` subtrees are licensed under ***Apache v2.0***.  
+ `common/libffi/` subtree is licensed under ***GNU GLP v2.0***.   

All the source code are licensed under ***MIT***. See ***COPYING*** for details.  

## **Contact**
Please contact me via the mail ***andy.zsshen@gmail.com***.  
Note that the kit is still under construction.  Contribution and bug report is desired.  


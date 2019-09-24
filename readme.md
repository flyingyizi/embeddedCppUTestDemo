
TDD是一种增量式软件开发技术，简单来说，就是在没有失败的单元测试前提下不可以写产品代码，这些测试要小，而且自动化。

重构：就是在不改变当前外部行为的条件下对现有代码结构进行修改的过程。其目的式通过写易于理解/演化并且易于我们维护的代码使得工作简单。

本例子演示了如何对嵌入式设备（NUCLEO-F103RB）编写UT，本例子的sample code是假设：
CppUTest source放置C:\prog\cpputest-3.8， 生成的libCppUTest.a放在C:\prog\cpputest-3.8\lib4nucleof103

下面将各个涉及步骤说明如下。 本demo参考了[搭配Atollic TrueSTUDIO尝试CppUTest](https://qiita.com/tk23ohtani/items/1f1cc4b9fa58a04f520c)


the first step was to build a library libCppUTest.a




1. git clone the latest cppUTest release
2. create a new static library in Atollic
3. file -> new -> c++ project -> static library -> embedded c++ library  using atollic ARM tools toolchains
4. select your target hardware ( e.g. nucleo-F103RB BOARD)
5. at same above UI, uncheck "disable c++ exception handlling" 
6. at next UI  uncheck release version, we only need debug version
7. from cpputest root directory, import src/CppUTest and src/CppUTestExt to your library's src folder
8. from cpputest root directory, import src/Platforms/lar  to your library's src folder
9. option: if select platforms/gcc, open the UtestPlatform.cpp you have just copied and substitute the lines 259-288 with lines 198-218 from src/Platforms/Keil/UtestPlatform.cpp (this make all mutex functions justdummies)   ----> or using lar's version
10. option:in your library's src folder edit IEEE754ExceptionsPlugin.cpp line 31 so it looks like this: `#ifdef CPPUTEST_USE_FENV ` (this turns off support for floating-point enviroment)
11. c/c++ build -> settings -> tool settings -> c++ compiler -> directories -> add `[CppUtestRoot]/include`
12. c/c++ build -> settings -> tool settings -> c++ compiler -> symbols -> add `CPPUTEST_STD_CPP_LIB_DISABLED`, `CPPUTEST_MEM_LEAK_DETECTION_DISABLED`
13. c/c++ build -> settings -> tool settings -> c++ compiler -> general -> c++ standard -> gnu++98
14. c/c++ build -> settings -> tool settings -> general -> runtime library -> rediced c and c++

this outputs a libCppUTest.a file in your library's debug folder.

now to crate a test project 

1. file -> new -> c++ project -> embedded c++ project 
2. select your target hardware ( e.g. nucleo-F103RB BOARD). it should match to libcpputest.a's target hardware, 
   如果是类似F4xx， RAM比较大的，建议选择RUN IN RAM.因为目的是TDD，则应经常对其进行测试，而对类似nucleof103RB板子，只有20KBRAM， 一个最基本带TEST的application都要编译到62KB，所以103板子在UTEST情况下玩RAM不合适。
3. uncheck "disable c++ exception handlling"
4. check generate system calls file (enable I/O..)
5. select your debugging tool (st-link in my case)
6. uncheck release 
7. c/c++ build -> settings -> tool settings -> c++ compiler -> directories -> add `[CppUtestRoot]/include`
8. c/c++ build -> settings -> tool settings -> c++ linker -> library search path -> `path_to_libCppUTest.a` 
9.  c/c++ build -> settings -> tool settings -> c++ linker -> libraries -> add  CppUTest (is "libCppUTest.a")
10. c/c++ build -> settings -> tool settings -> c++ compiler -> symbols -> add `CPPUTEST_STD_CPP_LIB_DISABLED`, `CPPUTEST_STD_C_LIB_DISABLED`, `CPPUTEST_MEM_LEAK_DETECTION_DISABLED`
11. add `#include` for your peripheral access layer into syscalls.c (stm32f10x in my case)
12. in syscalls.c change the body of _write to this (to handle console output)
    ```c
    int i;
    for(i=0;i<len;i++)  ITM_SendChar(*ptr++);
    return len;
    ```
    注意，由于ITM_SendChar是在core_cm3.h中定义的，建议直接使用下面形式之一
    ```c
    #include <main.h>
    //#include "stm32f10x.h"
    //#include "core_cm3.h"
    ```

13. if above select run in RAM, need to increase the HEAP size。编辑链接器命令文件stm32f4_ram.ld（放在FLASH上时是stm32f4_flash.ld。）
    例如：
    ```text
    stm32f4_ram.ld
    /* Generate a link error if heap and stack don't fit into RAM */
    _Min_Heap_Size  =  0x2000 ;  / *required amount of heap* / 
    _Min_Stack_Size  =  0x400 ;  / *required amount of stack* /
    ```

14. in debugger configuration (run -> debug configuration), new a lanuch configuration in "embedded c/c++ application".  enable SWV and set the core frequency to core frequency of your MCU
15. include `CppUTest/CommandLineTestRunner.h` in your main
16. add to the body of the main function
    ```c
    const char * av_override[]={};
    CommandLineTestRunner::RunAllTests(0,av_override);
    ```
17. create a tests.cpp file for your test
18. insert into tests.cpp
    ```c
    #include "CppUTest/TestHarness.h"
    TEST_GROUP(FirstTestGroup)
    {};
    TEST(FirstTestGroup, FirstTest)
    {
        FAIL("fail me!");
    }
    ```
19. start debug session
20. enable ITM port 0 in your SWV console, start tracing and resume run the debug. notes, nucleo-f103rb is 64Mhz, nucleo-f4 is 168Mhz
21. you should get follwing ouput in your SWV console, start tracing and run the debug
```text
..\src\test.cpp:16: error: Failure in TEST(FirstTestGroup, FirstTest)
	fail me!
.
Errors (1 failures, 1 tests, 1 ran, 1 checks, 0 ignored, 0 filtered out, 0 ms)
```


# 总结

> Bits, however, and computing systems at large, entail a consoling
> exception: in spite of their fantastic complexity, one can completely
> understand how modern computers work, and how they are built.

如名字一样，课程由与非门开始，搭建出一套可以玩俄罗斯方块的电脑。

课程高度专注于计算机体系结构中的核心概念，设计时只求实现功能，将优化、错误处理等内容留给更细分的专业领域。

课程分为上下两半，各6章。每章专注于计算机系统中的一个层次进行设计，下一层在前一层的基础上展开。

课程涵盖了计算机体系结构、操作系统、编译原理等多个领域的知识，是一门十分全面的计算机科学入门课程。

## 资源

[课程官网](https://www.nand2tetris.org/)

课程围绕作者自己写的教材进行设计([The Elements of Computing Systems](https://www.nand2tetris.org/book))。

21年刚刚再版了一次，原文语言简明易懂，国内也引入了中文翻译版本。

作者还做了一套教学视频在Coursera上。不过自己还是更喜欢读教材+做作业的方式。

课程内容完全自封闭，仅下半部分要求掌握一门高级语言作为前置知识，推荐零基础的朋友试试[CS50x](https://github.com/lng205/CS50x2022)。

配套内容十分完善，包括专门设计的硬件描述语言、测试脚本语言、算术逻辑单元、汇编语言、高级语言和操作系统，还有一整套仿真程序。

仿真程序2024年更新了[网页端IDE](https://nand2tetris.github.io/web-ide)，设计精美，免去了下载仿真程序的步骤。

课程所有相关内容均开源。赞美作者，赞美互联网精神。


## 上半部分

由逻辑门向上构建出数字电路模块，进而构建RAM, ROM, CPU，组合为一台通用计算机(称为HACK)，并在上面运行汇编语言。

课程对硬件的许多电气特性做了理想化处理，但设计的核心思想仍具普遍意义。

在学习时，结合电路知识，对硬件的物理特性进行了总结分析，使课程内容更具现实意义。

### [Week1](./01/readme.md)

### [Week2](./02/readme.md)

### [Week3](./03/readme.md)

### [Week4](./04/readme.md)

### [Week5](./05/readme.md)

### [Week6](./06/readme.md)


## 下半部分

> So, who are
> the good souls who turn high-level programming into an advanced
> technology indistinguishable from magic? They are the software wizards
> who develop compilers, virtual machines, and operating systems.

在HACK上进行软件开发，设计高级语言，虚拟机和操作系统，最终设计并运行俄罗斯方块这样的图形游戏。


### [Week7](./07/readme.md)

### [Week8](./08/readme.md)

### [Week9](./09/readme.md)

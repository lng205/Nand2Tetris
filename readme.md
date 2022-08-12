# 总结

[课程官网](https://www.nand2tetris.org/)

> Bits, however, and computing systems at large, entail a consoling
> exception: in spite of their fantastic complexity, one can completely
> understand how modern computers work, and how they are built. 

如名字一样，课程由非门开始，搭建出一套可以玩俄罗斯方块的电脑。

课程围绕作者自己写的教材进行设计([The Elements of Computing Systems](https://www.nand2tetris.org/book))。

21年刚刚再版了一次，Z-library可[下载](https://zh.book4you.org/book/15103557/a34b88)。

作者还做了一套教学视频在Coursera上。不过自己还是更喜欢读教材+做作业的方式。

课程高度专注于计算机体系结构中的核心概念，设计往往只求实现功能，将优化、差错控制等内容留给更细分的专业领域。

课程分为上下两半，每一章专注于计算机系统中的一个层次进行设计，下一层在前一层的基础上展开。



上半部分由逻辑门向上构建出数字电路模块，进而构建RAM, ROM, CPU，组合为一台通用计算机(称为HACK)，并在上面运行汇编语言。

课程对硬件的许多电气特性做了理想化处理，但设计的核心思想仍具普遍意义。

在学习时，结合电路知识，对硬件的物理特性进行了总结分析，使课程内容更具现实意义。



下半部分在HACK上进行软件开发，最终设计并运行俄罗斯方块这样的图形游戏。



课程内容完全自封闭，无需任何前置知识。

配套内容十分完善，包括专门设计的硬件描述语言、测试脚本语言、算术逻辑单元、汇编语言、高级语言和操作系统，还有一整套仿真程序。

课程的所有作业所需工具加起来不超过1M。

课程所有相关内容均开源。赞美作者，赞美互联网精神。
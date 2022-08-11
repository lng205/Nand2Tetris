# 三、时序逻辑

时序的概念使电路可以存储过去的状态，让电路中的反馈变得更加可控，门的翻转时间和传输时延导致的竞争冒险问题在边沿触发的时序电路中得到改善。

设计中开始多次出现对低层设计的复用。


## 计数器

用于实现程序指令的顺序执行。可通过置数实现跳转，通过清零使程序重新运行。

## 代码

PC模块在设计时需注意控制管脚优先级，第一个if对应的reset具有最高优先级，应该输入最后一级Mux。

RAM部分依序逐级构建即可。

## 电子电路复习

### 锁存器

将信号通过两个非门接回输入，就得到了一个可以保持自身状态的电路，称为锁存器。

将非门换成有两个输入端的或非或与非门，就得到了一个可以设置状态的锁存器，称为SR锁存器。两个外部输入端分别称为复位、置位端。


### 触发器

将SR锁存器的输入端与时钟信号相与，使其可受时钟控制，称为触发器，触发方式为电平触发。

将置位复位端通过非门合在一起，称为D触发器。

CMOS电路中，也可在非门环路的输入和反馈电路中放置两个互补控制的传输门，利用门的延时构成D触发器。

将两个电平触发的D触发器连在一起，输入相反的时钟信号，利用门电路的少量延时，可实现边沿触发的触发器。

将边沿触发器中的D触发器换成SR触发器，此时输入端可能在触发后回到保持态，构成脉冲触发的触发器。

可在脉冲触发器输入端加上输出端的反馈控制，使其在一个时钟周期内只可响应一次触发信号，构成JK触发器。


### 寄存器

多个D触发器并列，构成寄存器。实际往往还会加入保持、异步清零、三态输出等功能。保持功能可通过禁用时钟信号实现。


### RAM(Random Access Memory)

#### Static RAM/SRAM

原理与触发器相同，故速度最快，成本最高。常用作缓存(Cache)。


#### Dynamic RAM/DRAM

利用MOS电容存储，结构简单，集成度高。速度慢于SRAM，需要定期刷新。常用作内存(Memory)。


### ROM(Read-Only Memory)

经过发展，现有的ROM主要为采用闪存(Flash Memory)技术，可写，但写入速度远低于读取。常用于存放固件。

Flash的快速发展正使其在移动设备领域替代硬盘，SSD硬盘的原理与Flash有相似之处。
# 六、汇编器

> Since the translation from mnemonics and symbols to binary
> code is straightforward, it makes sense to write low-level programs directly
> in symbolic notation and have a computer program translate them into
> binary code.

## 总结

汇编器将汇编语言翻译成机器码，让人们免于直接通过机器码控制计算机。

汇编器的原理，在完成前五章之后，已显得较为清晰。汇编器的翻译过程实际上可以通过纸笔，以人工方式完成。如何将这个过程设计成程序，让计算机来实现，是本章的要点。

## 背景

除了对指令符号的映射外，汇编器的一个重要功能是对变量符号的支持。

汇编代码允许将程序中一行所在的位置定义为一个符号，也可以将数据内存的一个位置定义为一个符号，这提高了可读性，降低了开发难度。

符号的功能是通过汇编器中的符号表实现的。
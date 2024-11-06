#!python3.13

import os
import sys
import re
from enum import Enum


class CommandType(Enum):
    C_ARITHMETIC = 0
    C_PUSH = 1
    C_POP = 2
    C_LABEL = 3
    C_GOTO = 4
    C_IF = 5
    C_FUNCTION = 6
    C_RETURN = 7
    C_CALL = 8
    UNDEFINED = 9


class Parser:
    def __init__(self, file: str):
        self.f: list[str] = []
        with open(file) as file_in:
            for line in file_in:
                l = re.sub("//.*", "", line).strip()
                if l:
                    self.f.append(l.split(" "))
        self.index = 0

    def has_more_commands(self):
        return self.index < len(self.f)

    def advance(self):
        self.index += 1

    def command_type(self) -> CommandType:
        match self.f[self.index][0]:
            case "push":
                return CommandType.C_PUSH
            case "pop":
                return CommandType.C_POP
            case _:
                if self.f[self.index][0] in [
                    "add",
                    "sub",
                    "neg",
                    "eq",
                    "gt",
                    "lt",
                    "and",
                    "or",
                    "not",
                ]:
                    return CommandType.C_ARITHMETIC

    def arg1(self) -> str:
        assert self.command_type() != CommandType.C_RETURN
        if self.command_type() == CommandType.C_ARITHMETIC:
            return self.f[self.index][0]
        return self.f[self.index][1]

    def arg2(self) -> int:
        assert self.command_type() in [
            CommandType.C_PUSH,
            CommandType.C_POP,
            CommandType.C_FUNCTION,
            CommandType.C_CALL,
        ]
        return self.f[self.index][2]


class Writer:
    segment_map = {"local": "LCL", "argument": "ARG", "this": "THIS", "that": "THAT"}

    def __init__(self, filename: str):
        assert filename.endswith(".vm")
        self.file = open(f"{filename[:-3]}.asm", "w")
        self.filename = os.path.basename(filename)[:-3]
        self.label_count = 0

    def write_arithmetic(self, command: str):
        self.file.write(f"// {command}\n")
        commands = []
        match command:
            case "add":
                commands.extend(
                    [
                        "@SP",
                        "AM=M-1",
                        "D=M",
                        "A=A-1",
                        "M=D+M",
                    ]
                )
            case "sub":
                commands.extend(
                    [
                        "@SP",
                        "AM=M-1",
                        "D=M",
                        "A=A-1",
                        "M=M-D",
                    ]
                )
            case "neg":
                commands.extend(
                    [
                        "@SP",
                        "A=M-1",
                        "M=-M",
                    ]
                )
            case "eq":
                commands.extend(self._write_bool("JEQ"))
            case "gt":
                commands.extend(self._write_bool("JGT"))
            case "lt":
                commands.extend(self._write_bool("JLT"))
            case "and":
                commands.extend(
                    [
                        "@SP",
                        "AM=M-1",
                        "D=M",
                        "A=A-1",
                        "M=D&M",
                    ]
                )
            case "or":
                commands.extend(
                    [
                        "@SP",
                        "AM=M-1",
                        "D=M",
                        "A=A-1",
                        "M=D|M",
                    ]
                )
            case "not":
                commands.extend(
                    [
                        "@SP",
                        "A=M-1",
                        "M=!M",
                    ]
                )
        self.file.write("\n".join(commands) + "\n")

    def _write_bool(self, jump: str) -> list[str]:
        res = [
            "@SP",
            "AM=M-1",
            "D=M",
            "A=A-1",
            "D=M-D",
            f"@TRUE{self.label_count}",
            f"D;{jump}",
            # False
            "@SP",
            "A=M-1",
            "M=0",
            f"@CONTINUE{self.label_count}",
            "0;JMP",
            # True
            f"(TRUE{self.label_count})",
            "@SP",
            "A=M-1",
            "M=-1",
            # Continue
            f"(CONTINUE{self.label_count})",
        ]
        self.label_count += 1
        return res

    def write_push_pop(self, command_type: CommandType, segment: str, index: int):
        commands = []
        if command_type == CommandType.C_PUSH:
            commands.append(f"// push {segment} {index}")

            if segment == "constant":
                commands.extend(
                    [
                        f"@{index}",
                        "D=A",
                        # Push the value in D to the stack
                        "@SP",
                        "A=M",
                        "M=D",
                        "@SP",
                        "M=M+1",
                    ]
                )
            elif segment == "temp":
                commands.extend(
                    [
                        f"@R{5 + int(index)}",
                        "D=M",
                        "@SP",
                        "A=M",
                        "M=D",
                        "@SP",
                        "M=M+1",
                    ]
                )
            elif segment == "pointer":
                commands.extend(
                    [
                        "@THIS" if index == "0" else "@THAT",
                        "D=M",
                        "@SP",
                        "A=M",
                        "M=D",
                        "@SP",
                        "M=M+1",
                    ]
                )
            elif segment == "static":
                commands.extend(
                    [
                        f"@{self.filename}.{index}",
                        "D=M",
                        "@SP",
                        "A=M",
                        "M=D",
                        "@SP",
                        "M=M+1",
                    ]
                )
            else:
                assert segment in self.segment_map
                commands.extend(
                    [
                        f"@{self.segment_map[segment]}",
                        "D=M",
                        f"@{index}",
                        "A=D+A",
                        "D=M",
                        "@SP",
                        "A=M",
                        "M=D",
                        "@SP",
                        "M=M+1",
                    ]
                )

        elif command_type == CommandType.C_POP:
            commands.append(f"// pop {segment} {index}")

            if segment == "temp":
                commands.extend(
                    [
                        "@SP",
                        "AM=M-1",
                        "D=M",
                        f"@R{5 + int(index)}",
                        "M=D",
                    ]
                )
            elif segment == "pointer":
                commands.extend(
                    [
                        "@SP",
                        "AM=M-1",
                        "D=M",
                        "@THIS" if index == "0" else "@THAT",
                        "M=D",
                    ]
                )
            elif segment == "static":
                commands.extend(
                    [
                        "@SP",
                        "AM=M-1",
                        "D=M",
                        f"@{self.filename}.{index}",
                        "M=D",
                    ]
                )
            else:
                assert segment in self.segment_map
                commands.extend(
                    [
                        # Store the address of the segment base + index in R13
                        f"@{self.segment_map[segment]}",
                        "D=M",
                        f"@{index}",
                        "D=D+A",
                        "@R13",
                        "M=D",
                        # Pop the value from the stack and store it in D
                        "@SP",
                        "AM=M-1",
                        "D=M",
                        # Write D to the target address
                        "@R13",
                        "A=M",
                        "M=D",
                    ]
                )

        self.file.write("\n".join(commands) + "\n")

    def close(self):
        self.file.close()


def main():
    if len(sys.argv) != 2:
        sys.exit("Usage: python vm.py FILENAME.vm")

    parser = Parser(sys.argv[1])
    writer = Writer(sys.argv[1])
    while parser.has_more_commands():
        match parser.command_type():
            case CommandType.C_ARITHMETIC:
                writer.write_arithmetic(parser.arg1())
            case CommandType.C_PUSH | CommandType.C_POP:
                writer.write_push_pop(
                    parser.command_type(), parser.arg1(), parser.arg2()
                )
        parser.advance()
    writer.close()


if __name__ == "__main__":
    main()

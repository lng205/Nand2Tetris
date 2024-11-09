#!python3.13

import os
import sys
import re
from enum import Enum

WRITE_BOOTSTRAP = True


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
            case "label":
                return CommandType.C_LABEL
            case "goto":
                return CommandType.C_GOTO
            case "if-goto":
                return CommandType.C_IF
            case "function":
                return CommandType.C_FUNCTION
            case "call":
                return CommandType.C_CALL
            case "return":
                return CommandType.C_RETURN
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
                return CommandType.UNDEFINED

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
    push_D_commands = [
        "@SP",
        "A=M",
        "M=D",
        "@SP",
        "M=M+1",
    ]

    def __init__(self, filename: str):
        self.file = open(filename, "w")
        self.current_function = ""
        self.label_count = 0
        if WRITE_BOOTSTRAP:
            self._write_bootstrap()

    def _write_bootstrap(self):
        commands = [
            # SP = 256
            "@261",
            "D=A",
            "@SP",
            "M=D",
            # call Sys.init
            "@Sys.init",
            "0;JMP",
        ]
        self.file.write("\n".join(commands) + "\n")

    def set_file_name(self, filename: str):
        self.filename = filename

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
                commands.extend(self._bool_commands("JEQ"))
            case "gt":
                commands.extend(self._bool_commands("JGT"))
            case "lt":
                commands.extend(self._bool_commands("JLT"))
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

    def _bool_commands(self, jump: str) -> list[str]:
        # Helper function for eq, gt, lt
        res = [
            "@SP",
            "AM=M-1",
            "D=M",
            "A=A-1",
            "D=M-D",
            f"@TRUE.{self.label_count}",
            f"D;{jump}",
            # False
            "@SP",
            "A=M-1",
            "M=0",
            f"@CONTINUE.{self.label_count}",
            "0;JMP",
            # True
            f"(TRUE.{self.label_count})",
            "@SP",
            "A=M-1",
            "M=-1",
            # Continue
            f"(CONTINUE.{self.label_count})",
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
                    ]
                    + self.push_D_commands
                )
            elif segment == "temp":
                commands.extend(
                    [
                        f"@R{5 + int(index)}",
                        "D=M",
                    ]
                    + self.push_D_commands
                )
            elif segment == "pointer":
                commands.extend(
                    [
                        "@THIS" if index == "0" else "@THAT",
                        "D=M",
                    ]
                    + self.push_D_commands
                )
            elif segment == "static":
                commands.extend(
                    [
                        f"@{self.filename}.{index}",
                        "D=M",
                    ]
                    + self.push_D_commands
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
                    ]
                    + self.push_D_commands
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

    def write_label(self, label: str):
        self.file.write(f"({self.current_function}${label})\n")

    def write_goto(self, label: str):
        self.file.write(f"@{self.current_function}${label}\n0;JMP\n")

    def write_if(self, label: str):
        self.file.write(
            "\n".join(
                [
                    "@SP",
                    "AM=M-1",
                    "D=M",
                    f"@{self.current_function}${label}",
                    "D;JNE",
                ]
            )
            + "\n"
        )

    def write_function(self, function_name: str, n_vars: int):
        commands = [
            f"({function_name})",
        ] + [
            "@SP",
            "A=M",
            "M=0",
            "@SP",
            "M=M+1",
        ] * n_vars
        self.file.write("\n".join(commands) + "\n")
        self.current_function = function_name
        self.ret_counts = 0

    def write_call(self, function_name: str, n_args: int):
        self.file.write(f"// call {function_name} {n_args}\n")

        commands = (
            [
                # Push return address
                f"@{self.current_function}$ret.{self.ret_counts}",
                "D=A",
            ]
            + self.push_D_commands
            + [
                # Push LCL
                "@LCL",
                "D=M",
            ]
            + self.push_D_commands
            + [
                # Push ARG
                "@ARG",
                "D=M",
            ]
            + self.push_D_commands
            + [
                # Push THIS
                "@THIS",
                "D=M",
            ]
            + self.push_D_commands
            + [
                # Push THAT
                "@THAT",
                "D=M",
            ]
            + self.push_D_commands
            + [
                # ARG = SP - n - 5
                "@SP",
                "D=M",
                f"@{n_args + 5}",
                "D=D-A",
                "@ARG",
                "M=D",
                # LCL = SP
                "@SP",
                "D=M",
                "@LCL",
                "M=D",
                # goto f
                f"@{function_name}",
                "0;JMP",
                # (return address)
                f"({self.current_function}$ret.{self.ret_counts})",
            ]
        )
        self.file.write("\n".join(commands) + "\n")
        self.ret_counts += 1

    def write_return(self):
        self.file.write(f"// return\n")

        commands = [
            # frame(R13) = LCL
            "@LCL",
            "D=M",
            "@R13",
            "M=D",
            # retAddr(R14) = *(frame - 5)
            "@5",
            "A=D-A",
            "D=M",
            "@R14",
            "M=D",
            # *ARG = pop()
            "@SP",
            "AM=M-1",
            "D=M",
            "@ARG",
            "A=M",
            "M=D",
            # SP = ARG + 1
            "@ARG",
            "D=M+1",
            "@SP",
            "M=D",
            # THAT = *(frame - 1)
            "@R13",
            "AM=M-1",
            "D=M",
            "@THAT",
            "M=D",
            # THIS = *(frame - 2)
            "@R13",
            "AM=M-1",
            "D=M",
            "@THIS",
            "M=D",
            # ARG = *(frame - 3)
            "@R13",
            "AM=M-1",
            "D=M",
            "@ARG",
            "M=D",
            # LCL = *(frame - 4)
            "@R13",
            "AM=M-1",
            "D=M",
            "@LCL",
            "M=D",
            # goto ret
            "@R14",
            "A=M",
            "0;JMP",
        ]
        self.file.write("\n".join(commands) + "\n")

    def close(self):
        self.file.close()


def main():
    if len(sys.argv) != 2:
        sys.exit("Usage: python vm.py FILENAME.vm or python vm.py DIRECTORY")

    path = sys.argv[1]
    if os.path.isdir(path):
        files = [os.path.join(path, f) for f in os.listdir(path) if f.endswith(".vm")]
        assert files
        writer = Writer(os.path.join(path, os.path.basename(path) + ".asm"))
    else:
        files = [path]
        assert path.endswith(".vm")
        writer = Writer(path[:-3] + ".asm")

    for file in files:
        parser = Parser(file)
        writer.set_file_name(os.path.basename(file)[:-3])
        while parser.has_more_commands():
            match parser.command_type():
                case CommandType.C_ARITHMETIC:
                    writer.write_arithmetic(parser.arg1())
                case CommandType.C_PUSH | CommandType.C_POP:
                    writer.write_push_pop(
                        parser.command_type(), parser.arg1(), parser.arg2()
                    )
                case CommandType.C_LABEL:
                    writer.write_label(parser.arg1())
                case CommandType.C_GOTO:
                    writer.write_goto(parser.arg1())
                case CommandType.C_IF:
                    writer.write_if(parser.arg1())
                case CommandType.C_FUNCTION:
                    writer.write_function(parser.arg1(), int(parser.arg2()))
                case CommandType.C_CALL:
                    writer.write_call(parser.arg1(), int(parser.arg2()))
                case CommandType.C_RETURN:
                    writer.write_return()
            parser.advance()
    writer.close()


if __name__ == "__main__":
    main()

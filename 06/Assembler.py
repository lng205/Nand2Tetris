import sys
import re

def main():
    if len(sys.argv) != 2:
        sys.exit("Usage: python Assembler.py FILE")

    #  Remove comments, spaces and empty lines. Build label table
    with open(sys.argv[1]) as file_in:
        f = []
        line_counter = 0
        for line in file_in:
            l = re.sub("//.*|\s", "", line).strip()
            if l:
                if l[0] == '(':
                    syms[l[1:-1]] = line_counter
                    continue
                f.append(l)
                line_counter += 1

    # Translate and write
    with open(sys.argv[1][0:-4] + '1.hack', 'w') as file_out:
        symbol_counter = 16
        for line in f:
            if line[0] == '@':
                # A-instruction
                addr = line[1:]
                if addr in syms:
                    addr_value = syms[addr]
                elif addr.isdigit():
                    addr_value = int(addr)
                else:
                    addr_value = symbol_counter
                    syms[addr] = symbol_counter
                    symbol_counter += 1
                file_out.write(f"{addr_value:016b}\n")
            else:
                # C-instruction
                calc = '111'
                p1, p2 = line.rfind('='), line.rfind(';')
                if p1 >= 0:
                    if p2 >= 0:
                        # dest=comp;jump
                        calc += comp[line[p1 + 1:p2]] + dest[line[0:p1]] + jump[line[p2 + 1:]]
                    else:
                        # dest=comp
                        calc += comp[line[p1 + 1:]] + dest[line[0:p1]] + '000'
                else:
                    # comp;jump
                    calc += comp[line[0:p2]] + '000' + jump[line[p2 + 1:]]
                file_out.write(calc + '\n')


syms = {'SP': 0, 'LCL': 1, 'ARG': 2, 'THIS': 3, 'THAT': 4, 'SCREEN': 16384, 'KBD': 24576,
        'R0': 0, 'R1': 1, 'R2': 2, 'R3': 3, 'R4': 4, 'R5': 5, 'R6': 6, 'R7': 7, 'R8': 8,
        'R9': 9, 'R10': 10, 'R11': 11, 'R12': 12, 'R13': 13, 'R14': 14, 'R15': 15}

comp = {'0'  : '0101010', '1'  : '0111111', '-1' : '0111010',
        'D'  : '0001100', 'A'  : '0110000', 'M'  : '1110000',
        '!D' : '0001101', '!A' : '0110001', '!M' : '1110001',
        '-D' : '0001111', '-A' : '0110011', '-M' : '1110011',
        'D+1': '0011111', 'A+1': '0110111', 'M+1': '1110111',
        'D-1': '0001110', 'A-1': '0110010', 'M-1': '1110010',
        'D+A': '0000010', 'D+M': '1000010', 'D-A': '0000010',
        'D-A': '0010011', 'D-M': '1010011', 'A-D': '0000111',
        'M-D': '1000111', 'D&A': '0000000', 'D&M': '1000000',
        'D|A': '0010101', 'D|M': '1010101'}

dest = {'M': '001', 'D': '010', 'DM': '011', 'MD': '011', 'A': '100',
        'AM': '101', 'AD': '110', 'ADM': '111'}

jump = {'JGT': '001', 'JEQ': '010', 'JGE': '011', 'JLT': '100',
        'JNE': '101', 'JLE': '110', 'JMP': '111'}

main()
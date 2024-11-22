import sys
import os
from JackTokenizer import JackTokenizer
from CompilationEngine import CompilationEngine
from utils import tokenType, keyWord


def main():
    path = sys.argv[1]
    if os.path.isdir(path):
        files = [os.path.join(path, file) for file in os.listdir(path) if file.endswith(".jack")]
    else:
        files = [path]

    for file in files:
        file_in = open(file, "r")
        file_out = open(file.replace(".jack", "T.xml"), "w")
        file_out.write("<tokens>\n")

        tokenizer = JackTokenizer(file_in)
        while tokenizer.hasMoreTokens():
            tokenizer.advance()
            match tokenizer.tokenType():
                case tokenType.KEYWORD:
                    file_out.write(f"<keyword> {tokenizer.keyWord()} </keyword>\n")
                case tokenType.SYMBOL:
                    symbol = tokenizer.symbol()
                    if symbol == "<":
                        symbol = "&lt;"
                    elif symbol == ">":
                        symbol = "&gt;"
                    elif symbol == "&":
                        symbol = "&amp;"
                    file_out.write(f"<symbol> {symbol} </symbol>\n")
                case tokenType.IDENTIFIER:
                    file_out.write(
                        f"<identifier> {tokenizer.identifier()} </identifier>\n"
                    )
                case tokenType.INT_CONST:
                    file_out.write(
                        f"<integerConstant> {tokenizer.intVal()} </integerConstant>\n"
                    )
                case tokenType.STRING_CONST:
                    file_out.write(
                        f"<stringConstant> {tokenizer.stringVal()} </stringConstant>\n"
                    )
        file_out.write("</tokens>\n")
        file_in.close()
        file_out.close()


if __name__ == "__main__":
    main()

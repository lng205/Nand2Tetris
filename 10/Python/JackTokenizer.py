import re
from utils import tokenType, keyWord
from typing import IO

SYMBOL = "{}()[].,;+-*/&|<>=~"
KEYWORDS = {kw.name.lower() for kw in keyWord}

TOKEN_PATTERNS = {
    "COMMENT": r"//.*|/\*.*?\*/|/\*\*.?\*/",
    "KEYWORD": "|".join(KEYWORDS),
    "SYMBOL": "[" + re.escape(SYMBOL) + "]",
    "IDENTIFIER": "[a-zA-Z_][a-zA-Z0-9_]*",
    "INT_CONST": r"\d+",
    "STRING_CONST": '"[^"\n]*"',
}

PATTERNS = re.compile(
    "|".join(f"(?P<{name}>{pattern})" for name, pattern in TOKEN_PATTERNS.items())
)


class JackTokenizer:
    def __init__(self, file: IO[str]):
        self.file = file.read()
        self.tokens = [match for match in PATTERNS.finditer(self.file)]
        self.current_index = -1
        self.current_token = None

    def hasMoreTokens(self) -> bool:
        return self.current_index + 1 < len(self.tokens)

    def advance(self) -> None:
        self.current_index += 1
        self.current_token = self.tokens[self.current_index]
        if self.current_token.lastgroup == "COMMENT":
            self.advance()

    def tokenType(self) -> tokenType:
        return tokenType[self.current_token.lastgroup]

    def keyWord(self) -> str:
        return self.current_token.group()

    def symbol(self) -> str:
        return self.current_token.group()

    def identifier(self) -> str:
        return self.current_token.group()

    def intVal(self) -> int:
        return int(self.current_token.group())

    def stringVal(self) -> str:
        return self.current_token.group()[1:-1]

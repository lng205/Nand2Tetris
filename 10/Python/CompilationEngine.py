from typing import IO

class CompilationEngine:
    def __init__(self, file_in: IO[str], file_out: IO[str]):
        self.file_in = file_in
        self.file_out = file_out

#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include "CompilationEngine.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    std::string path;
    if (argc < 2) {
        path = ".";
    } else {
        path = argv[1];
    }

    std::vector<std::string> files;
    if (fs::is_directory(path)) {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.path().extension() == ".jack") {
                files.push_back(entry.path().string());
            }
        }
    } else {
        files.push_back(path);
    }

    for (const auto& file : files) {
        std::ifstream input(file);
        JackTokenizer tokenizer(input);
        std::ofstream output(file.substr(0, file.find_last_of('.')) + ".vm");
        CompilationEngine engine(tokenizer, output);

        tokenizer.advance();
        if (tokenizer.keyWord() == "class") {
            engine.compileClass();
        }
    }
    return 0;
}
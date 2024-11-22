#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include "JackTokenizer.h"

namespace fs = std::filesystem;

std::string escapeXML(char ch) {
    switch (ch) {
        case '<':
            return "&lt;";
        case '>':
            return "&gt;";
        case '&':
            return "&amp;";
        default:
            return std::string(1, ch);
    }
}

int main(int argc, char* argv[]) {
    std::string path = argv[1];
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
        std::ofstream output(file.substr(0, file.find_last_of('.')) + "_read.xml");
        output << "<tokens>\n";

        JackTokenizer tokenizer(input);
        while (tokenizer.hasMoreTokens()) {
            tokenizer.advance();
            switch (tokenizer.tokenType()) {
                case TokenType::KEYWORD:
                    output << "<keyword> " << tokenizer.keyWord() << " </keyword>\n";
                    break;
                case TokenType::SYMBOL:

                    output << "<symbol> " << escapeXML(tokenizer.symbol()) << " </symbol>\n";
                    break;
                case TokenType::IDENTIFIER:
                    output << "<identifier> " << tokenizer.identifier() << " </identifier>\n";
                    break;
                case TokenType::INT_CONST:
                    output << "<integerConstant> " << tokenizer.intVal() << " </integerConstant>\n";
                    break;
                case TokenType::STRING_CONST:
                    output << "<stringConstant> " << tokenizer.stringVal() << " </stringConstant>\n";
                    break;
                default:
                    break;
            }
        }
        output << "</tokens>\n";
        input.close();
    }
    return 0;
}
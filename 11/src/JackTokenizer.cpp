#include "JackTokenizer.h"
#include <algorithm>

JackTokenizer::JackTokenizer(std::ifstream &input)
    : file(input), currentTokenType(TokenType::UNKNOWN), currentToken("") {};

bool JackTokenizer::hasMoreTokens() {
    return !file.eof();
}

void JackTokenizer::advance() {
    currentTokenType = TokenType::UNKNOWN;
    currentToken.clear();
    char ch;

    // skip whitespace
    while (file.get(ch) && isspace(ch));
    if (file.eof()) {
        return;
    }

    // skip comments
    if (ch == '/') {
        char nextCh = file.peek();
        if (nextCh == '/') {
            // single line comment
            while (file.get(ch) && ch != '\n');
            return advance();
        }
        else if (nextCh == '*') {
            // multi-line comment
            file.get(ch);
            while (file.get(ch)) {
                if (ch == '*' && file.peek() == '/') {
                    file.get(ch);
                    return advance();
                }
            }
            throw std::runtime_error("Unterminated comment");
        }
    }

    // match string constant
    if (ch == '"') {
        currentTokenType = TokenType::STRING_CONST;
        currentToken = "";
        while (file.get(ch)) {
            if (ch == '"') {
                break;
            }
            if (ch == '\n' || file.eof()) {
                throw std::runtime_error("Unterminated string constant");
            }
            currentToken += ch;
        }
    }
    // match integer constant in the range 0..32767
    else if (isdigit(ch)) {
        currentTokenType = TokenType::INT_CONST;
        currentToken = ch;
        while (file.get(ch) && isdigit(ch)) {
            currentToken += ch;
        }
        file.unget();
        int intVal = std::stoi(currentToken);
        if (intVal < 0 || intVal > 32767) {
            throw std::runtime_error("Integer constant out of range");
        }
    }
    // match keyword or identifier
    else if (isalpha(ch) || ch == '_') {
        currentToken = ch;
        while (file.get(ch) && (isalnum(ch) || ch == '_')) {
            currentToken += ch;
        }
        file.unget();
        if (keywords.find(currentToken) != keywords.end()) {
            currentTokenType = TokenType::KEYWORD;
        }
        else {
            currentTokenType = TokenType::IDENTIFIER;
        }
    }
    // match symbols
    else if (symbols.find(ch) != symbols.end()) {
        currentTokenType = TokenType::SYMBOL;
        currentToken = ch;
    }
}

TokenType JackTokenizer::tokenType() {
    return currentTokenType;
}

std::string JackTokenizer::keyWord() {
    return currentToken;
}

char JackTokenizer::symbol() {
    return currentToken[0];
}

std::string JackTokenizer::identifier() {
    return currentToken;
}

int JackTokenizer::intVal() {
    return std::stoi(currentToken);
}

std::string JackTokenizer::stringVal() {
    return currentToken;
}
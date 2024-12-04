#include <fstream>
#include <unordered_set>

enum TokenType {
    KEYWORD,
    SYMBOL,
    IDENTIFIER,
    INT_CONST,
    STRING_CONST,
    UNKNOWN
};

class JackTokenizer {
public:
    JackTokenizer(std::ifstream &input);
    bool hasMoreTokens();
    void advance();
    TokenType tokenType();
    std::string keyWord();
    char symbol();
    std::string identifier();
    int intVal();
    std::string stringVal();

private:
    std::ifstream &file;
    TokenType currentTokenType;
    std::string currentToken;
    const std::unordered_set<char> symbols = {
        '{', '}', '(', ')', '[', ']', '.', ',', ';', 
        '+', '-', '*', '/', '&', '|', '<', '>', '=', '~'
    };
    const std::unordered_set<std::string> keywords = {
        "class", "constructor", "function", "method", "field", "static", 
        "var", "int", "char", "boolean", "void", "true", "false", "null", 
        "this", "let", "do", "if", "else", "while", "return"
    };
};
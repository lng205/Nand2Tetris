#include "CompilationEngine.h"

CompilationEngine::CompilationEngine(JackTokenizer &input, std::ofstream &output) : tokenizer(input), output(output) {};

void CompilationEngine::compileClass()
{
    output << "<class>\n";
    tokenizer.advance();
    process("class");
    process(tokenizer.identifier());
    process("{");
    // classVar must precede subroutine
    // classVarDec*
    while (tokenizer.tokenType() == TokenType::KEYWORD && (tokenizer.keyWord() == "static" || tokenizer.keyWord() == "field"))
    {
        compileClassVarDec();
    }
    // subroutineDec*
    while (tokenizer.tokenType() == TokenType::KEYWORD && (tokenizer.keyWord() == "constructor" || tokenizer.keyWord() == "function" || tokenizer.keyWord() == "method"))
    {
        compileSubroutine();
    }
    process("}");
    output << "</class>\n";
}

void CompilationEngine::process(std::string str) {
    switch (tokenizer.tokenType()) {
        case TokenType::KEYWORD:
            if (tokenizer.keyWord() == str) {
                output << "<keyword> " << str << " </keyword>\n";
            }
            break;
        case TokenType::SYMBOL:
            if (tokenizer.symbol() == str[0]) {
                if (str == "<") {
                    output << "<symbol> &lt; </symbol>\n";
                } else if (str == ">") {
                    output << "<symbol> &gt; </symbol>\n";
                } else if (str == "&") {
                    output << "<symbol> &amp; </symbol>\n";
                } else {
                    output << "<symbol> " << str << " </symbol>\n";
                }
            }
            break;
        case TokenType::IDENTIFIER:
            if (tokenizer.identifier() == str) {
                output << "<identifier> " << str << " </identifier>\n";
            }
            break;
        case TokenType::STRING_CONST:
            if (tokenizer.stringVal() == str) {
                output << "<stringConstant> " << str << " </stringConstant>\n";
            }
            break;
        default:
            break;
    }
    tokenizer.advance();
}

void CompilationEngine::processType() {
    if (tokenizer.tokenType() == TokenType::KEYWORD) {
        // "int", "char", "boolean"
        process(tokenizer.keyWord());
    } else {
        // className
        process(tokenizer.identifier());
    }
}

void CompilationEngine::compileClassVarDec() {
    output << "<classVarDec>\n";
    // "static" or "field"
    process(tokenizer.keyWord());
    // type
    processType();
    // varName
    process(tokenizer.identifier());
    // (',' varName)*
    while (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
        process(",");
        process(tokenizer.identifier());
    }
    process(";");
    output << "</classVarDec>\n";
}

void CompilationEngine::compileSubroutine() {
    // subroutineDec
    output << "<subroutineDec>\n";
    // "constructor" or "function" or "method"
    process(tokenizer.keyWord());
    // "void" or type
    if (tokenizer.tokenType() == TokenType::KEYWORD && tokenizer.keyWord() == "void") {
        process("void");
    } else {
        processType();
    }
    // subroutineName
    process(tokenizer.identifier());
    process("(");
    compileParameterList();
    process(")");
    compileSubroutineBody();
    output << "</subroutineDec>\n";
}

void CompilationEngine::compileParameterList() {
    output << "<parameterList>\n";
    // ((type varName) (',' type varName)*)?
    // FOLLOW(parameterList) = {")"}
    if (tokenizer.tokenType() != TokenType::SYMBOL) {
        processType();
        // varName
        process(tokenizer.identifier());
        // (',' type varName)*
        while (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
            process(",");
            processType();
            process(tokenizer.identifier());
        }
    }
    output << "</parameterList>\n";
}

void CompilationEngine::compileSubroutineBody() {
    output << "<subroutineBody>\n";
    process("{");
    // varDec*
    while (tokenizer.tokenType() == TokenType::KEYWORD && tokenizer.keyWord() == "var") {
        compileVarDec();
    }
    compileStatements();
    process("}");
    output << "</subroutineBody>\n";
}

void CompilationEngine::compileVarDec() {
    output << "<varDec>\n";
    // "var"
    process("var");
    // type
    processType();
    // varName
    process(tokenizer.identifier());
    // (',' varName)*
    while (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
        process(",");
        process(tokenizer.identifier());
    }
    process(";");
    output << "</varDec>\n";
}

void CompilationEngine::compileStatements() {
    output << "<statements>\n";
    while (tokenizer.tokenType() == TokenType::KEYWORD) {
        if (tokenizer.keyWord() == "let") {
            compileLet();
        } else if (tokenizer.keyWord() == "if") {
            compileIf();
        } else if (tokenizer.keyWord() == "while") {
            compileWhile();
        } else if (tokenizer.keyWord() == "do") {
            compileDo();
        } else if (tokenizer.keyWord() == "return") {
            compileReturn();
        } else {
            break;
        }
    }
    output << "</statements>\n";
}

void CompilationEngine::compileLet() {
    output << "<letStatement>\n";
    process("let");
    // varName
    process(tokenizer.identifier());
    // array
    // ('[' expression ']')?
    if (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == '[') {
        process("[");
        compileExpression();
        process("]");
    }
    process("=");
    compileExpression();
    process(";");
    output << "</letStatement>\n";
}

void CompilationEngine::compileIf() {
    output << "<ifStatement>\n";
    process("if");
    process("(");
    compileExpression();
    process(")");
    process("{");
    compileStatements();
    process("}");
    // else
    if (tokenizer.tokenType() == TokenType::KEYWORD && tokenizer.keyWord() == "else") {
        process("else");
        process("{");
        compileStatements();
        process("}");
    }
    output << "</ifStatement>\n";
}

void CompilationEngine::compileWhile() {
    output << "<whileStatement>\n";
    process("while");
    process("(");
    compileExpression();
    process(")");
    process("{");
    compileStatements();
    process("}");
    output << "</whileStatement>\n";
}

void CompilationEngine::compileDo() {
    output << "<doStatement>\n";
    process("do");
    // consume the first identifier
    process(tokenizer.identifier());
    compileSubroutineCall();
    process(";");
    output << "</doStatement>\n";
}

void CompilationEngine::compileReturn() {
    output << "<returnStatement>\n";
    process("return");
    // expression?
    if (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ';') {
        process(";");
    } else {
        compileExpression();
        process(";");
    }
    output << "</returnStatement>\n";
}

void CompilationEngine::compileExpression() {
    output << "<expression>\n";
    compileTerm();
    // (op term)*
    while (tokenizer.tokenType() == TokenType::SYMBOL && opSet.find(std::string{tokenizer.symbol()}) != opSet.end()) {
        process(std::string{tokenizer.symbol()});
        compileTerm();
    }
    output << "</expression>\n";
}

void CompilationEngine::compileTerm() {
    output << "<term>\n";
    switch (tokenizer.tokenType()) {
        case TokenType::INT_CONST:
            output << "<integerConstant> " << tokenizer.intVal() << " </integerConstant>\n";
            tokenizer.advance();
            break;
        case TokenType::STRING_CONST:
            process(tokenizer.stringVal());
            break;
        case TokenType::KEYWORD:
            if (tokenizer.keyWord() == "true" || tokenizer.keyWord() == "false" || tokenizer.keyWord() == "null" || tokenizer.keyWord() == "this") {
                process(tokenizer.keyWord());
            }
            break;
        case TokenType::IDENTIFIER:
            // varName | varName '[' expression ']' | subroutineCall
            process(tokenizer.identifier());
            if (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == '[') {
                process("[");
                compileExpression();
                process("]");
            } else if (tokenizer.tokenType() == TokenType::SYMBOL && (tokenizer.symbol() == '(' || tokenizer.symbol() == '.')) {
                compileSubroutineCall();
            }
            break;
        case TokenType::SYMBOL:
            if (tokenizer.symbol() == '(') {
                // '(' expression ')'
                process("(");
                compileExpression();
                process(")");

            } else if (opSet.find(std::string{tokenizer.symbol()}) != opSet.end()) {
                // unaryOp term
                process(std::string{tokenizer.symbol()});
                compileTerm();
            }
    };
    output << "</term>\n";
}

void CompilationEngine::compileSubroutineCall() {
    // subroutineName '(' expressionList ')' | (className | varName) '.' subroutineName '(' expressionList ')'
    // the first identifier has been consumed to determine the type of the term
    if (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == '.') {
        process(".");
        process(tokenizer.identifier());
    }
    process("(");
    compileExpressionList();
    process(")");
}

int CompilationEngine::compileExpressionList() {
    output << "<expressionList>\n";
    int nArgs = 0;
    // (expression (',' expression)*)?
    if (tokenizer.tokenType() != TokenType::SYMBOL || tokenizer.symbol() != ')') {
        compileExpression();
        nArgs++;
        while (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
            process(",");
            compileExpression();
            nArgs++;
        }
    }
    output << "</expressionList>\n";
    return nArgs;
}
#include "CompilationEngine.h"

CompilationEngine::CompilationEngine(JackTokenizer &input, std::ofstream &output) : tokenizer(input), writer(output) {};

void CompilationEngine::process(std::string str) {
    switch (tokenizer.tokenType()) {
        case TokenType::KEYWORD:
            if (tokenizer.keyWord() == str) {
            }
            break;
        case TokenType::SYMBOL:
            if (tokenizer.symbol() == str[0]) {
            }
            break;
        case TokenType::IDENTIFIER:
            if (tokenizer.identifier() == str) {
            }
            break;
        case TokenType::STRING_CONST:
            if (tokenizer.stringVal() == str) {
            }
            break;
        default:
            break;
    }
    tokenizer.advance();
}

std::string CompilationEngine::processType() {
    std::string type;
    // "int" or "char" or "boolean" or className
    type = tokenizer.tokenType() == TokenType::KEYWORD ? tokenizer.keyWord() : tokenizer.identifier();
    process(type);
    return type;
}

void CompilationEngine::compileClass()
{
    classSyms.reset();
    labelCount = 0;

    tokenizer.advance();
    process("class");

    // className
    className = tokenizer.identifier();
    process(tokenizer.identifier());

    process("{");
    // classVar must precede subroutine
    while (tokenizer.tokenType() == TokenType::KEYWORD && (tokenizer.keyWord() == "static" || tokenizer.keyWord() == "field"))
    {
        // classVarDec*
        compileClassVarDec();
    }
    while (tokenizer.tokenType() == TokenType::KEYWORD && (tokenizer.keyWord() == "constructor" || tokenizer.keyWord() == "function" || tokenizer.keyWord() == "method"))
    {
        // subroutineDec*
        compileSubroutine();
    }
    process("}");
}

void CompilationEngine::compileClassVarDec() {
    // "static" or "field"
    Kind kind = tokenizer.keyWord() == "static" ? Kind::STATIC : Kind::FIELD;
    process(tokenizer.keyWord());
    // type
    std::string type = processType();
    // varName
    classSyms.define(tokenizer.identifier(), type, kind);
    process(tokenizer.identifier());
    while (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
        // (',' varName)*
        process(",");
        classSyms.define(tokenizer.identifier(), type, kind);
        process(tokenizer.identifier());
    }
    process(";");
}

void CompilationEngine::compileSubroutine() {
    subroutineSyms.reset();
    // "constructor" or "function" or "method"
    subroutineType = tokenizer.keyWord();
    process(tokenizer.keyWord());

    if (tokenizer.tokenType() == TokenType::KEYWORD && tokenizer.keyWord() == "void") {
        process("void");
    } else {
        processType();
    }

    // subroutineName
    subroutineName = tokenizer.identifier();
    if (subroutineType == "method") {
        subroutineSyms.define("this", subroutineName, Kind::ARG);
    }
    process(tokenizer.identifier());

    process("(");
    compileParameterList();
    process(")");

    compileSubroutineBody();
}

void CompilationEngine::compileParameterList() {
    // FOLLOW(parameterList) = {")"}
    if (tokenizer.tokenType() != TokenType::SYMBOL) {
        // ((type varName) (',' type varName)*)?
        std::string type = processType();
        // varName
        subroutineSyms.define(tokenizer.identifier(), type, Kind::ARG);
        process(tokenizer.identifier());
        while (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
            // (',' type varName)*
            process(",");
            type = processType();
            subroutineSyms.define(tokenizer.identifier(), type, Kind::ARG);
            process(tokenizer.identifier());
        }
    }
}

void CompilationEngine::compileSubroutineBody() {
    process("{");
    // varDec*
    while (tokenizer.tokenType() == TokenType::KEYWORD && tokenizer.keyWord() == "var") {
        compileVarDec();
    }
    writer.writeFunction(className + "." + subroutineName, subroutineSyms.varCount(Kind::VAR));
    if (subroutineType == "method") {
        writer.writePush("argument", 0);
        writer.writePop("pointer", 0);
    } else if (subroutineType == "constructor") {
        writer.writePush("constant", classSyms.varCount(Kind::FIELD));
        writer.writeCall("Memory.alloc", 1);
        writer.writePop("pointer", 0);
    }

    compileStatements();
    process("}");
}

void CompilationEngine::compileVarDec() {
    // "var"
    process("var");
    // type
    std::string type = processType();
    // varName
    subroutineSyms.define(tokenizer.identifier(), type, Kind::VAR);
    process(tokenizer.identifier());
    while (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
        // (',' varName)*
        process(",");
        subroutineSyms.define(tokenizer.identifier(), type, Kind::VAR);
        process(tokenizer.identifier());
    }
    process(";");
}

void CompilationEngine::compileStatements() {
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
}

void CompilationEngine::compileLet() {
    process("let");
    // varName
    std::string varName = tokenizer.identifier();
    process(tokenizer.identifier());
    std::string varKind;
    int varIndex;
    if (subroutineSyms.kindOf(varName) != "NONE") {
        varKind = subroutineSyms.kindOf(varName);
        varIndex = subroutineSyms.indexOf(varName);
    }
    else {
        varKind = classSyms.kindOf(varName);
        varIndex = classSyms.indexOf(varName);
    }

    bool isArray = (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == '[');
    if (isArray) {
        writer.writePush(varKind, varIndex);

        // ('[' expression ']')?
        process("[");
        compileExpression();
        process("]");

        // calculate the address
        writer.writeArithmetic("add");
    }
    process("=");
    compileExpression();
    if (isArray) {
        writer.writePop("temp", 0);
        writer.writePop("pointer", 1);
        writer.writePush("temp", 0);
        writer.writePop("that", 0);
    } else {
        writer.writePop(varKind, varIndex);
    }
    process(";");
}

void CompilationEngine::compileIf() {
    process("if");

    process("(");
    compileExpression();
    process(")");
    writer.writeArithmetic("not");
    writer.writeIf("L" + std::to_string(labelCount));

    process("{");
    compileStatements();
    process("}");
    writer.writeGoto("L" + std::to_string(labelCount + 1));

    // else
    if (tokenizer.tokenType() == TokenType::KEYWORD && tokenizer.keyWord() == "else") {
        writer.writeLabel("L" + std::to_string(labelCount));
        process("else");
        process("{");
        compileStatements();
        process("}");
    }
    writer.writeLabel("L" + std::to_string(labelCount + 1));
    labelCount += 2;
}

void CompilationEngine::compileWhile() {
    process("while");
    process("(");
    compileExpression();
    process(")");
    process("{");
    compileStatements();
    process("}");
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
        case TokenType::UNKNOWN:
            break;
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
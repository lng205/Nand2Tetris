#include "CompilationEngine.h"

CompilationEngine::CompilationEngine(JackTokenizer &input, std::ofstream &output) : tokenizer(input), writer(output) {};

void CompilationEngine::compileClass()
{
    classSyms.reset();
    labelCount = 0;

    process("class");

    // className
    className = tokenizer.identifier();
    tokenizer.advance();

    // classVarDec must precede subroutine
    process("{");
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
    // ("static" | "field") type varName (',' varName)* ';'

    // "static" or "field"
    Kind kind = tokenizer.keyWord() == "static" ? Kind::STATIC : Kind::FIELD;
    tokenizer.advance();

    // type
    std::string type = processType();

    // varName
    classSyms.define(tokenizer.identifier(), type, kind);
    tokenizer.advance();

    while (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
        // (',' varName)*
        process(",");
        // varName
        classSyms.define(tokenizer.identifier(), type, kind);
        tokenizer.advance();
    }
    process(";");
}

void CompilationEngine::compileSubroutine() {
    // ("constructor" | "function" | "method") ("void" | type) subroutineName '(' parameterList ')' subroutineBody

    subroutineSyms.reset();

    // ("constructor" | "function" | "method")
    subroutineType = tokenizer.keyWord();
    tokenizer.advance();

    // ("void" | type)
    tokenizer.advance();

    // subroutineName
    subroutineName = tokenizer.identifier();
    if (subroutineType == "method") {
        subroutineSyms.define("this", className, Kind::ARG);
    }
    tokenizer.advance();

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
        tokenizer.advance();
        while (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
            // (',' type varName)*
            process(",");
            type = processType();
            subroutineSyms.define(tokenizer.identifier(), type, Kind::ARG);
            tokenizer.advance();
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
    tokenizer.advance();
    while (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
        // (',' varName)*
        process(",");
        subroutineSyms.define(tokenizer.identifier(), type, Kind::VAR);
        tokenizer.advance();
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
    tokenizer.advance();
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

    writer.writeLabel("L" + std::to_string(labelCount));
    if (tokenizer.tokenType() == TokenType::KEYWORD && tokenizer.keyWord() == "else") {
        // else
        process("else");
        process("{");
        compileStatements();
        process("}");
    }
    writer.writeLabel("L" + std::to_string(labelCount + 1));
    labelCount += 2;
}

void CompilationEngine::compileWhile() {
    writer.writeLabel("L" + std::to_string(labelCount));
    process("while");

    process("(");
    compileExpression();
    process(")");
    writer.writeArithmetic("not");
    writer.writeIf("L" + std::to_string(labelCount + 1));

    process("{");
    compileStatements();
    process("}");
    writer.writeGoto("L" + std::to_string(labelCount));

    writer.writeLabel("L" + std::to_string(labelCount + 1));
    labelCount += 2;
}

void CompilationEngine::compileDo() {
    process("do");
    // consume the first identifier
    std::string token = tokenizer.identifier();
    tokenizer.advance();
    compileSubroutineCall(token);

    writer.writePop("temp", 0);
    process(";");
}

void CompilationEngine::compileReturn() {
    process("return");
    // expression?
    if (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ';') {
        // return void
        writer.writePush("constant", 0);
    } else {
        compileExpression();
    }
    writer.writeReturn();

    process(";");
}

void CompilationEngine::compileExpression() {
    compileTerm();
    // (op term)*
    while (tokenizer.tokenType() == TokenType::SYMBOL && opSet.find(std::string{tokenizer.symbol()}) != opSet.end()) {
        char op = tokenizer.symbol();
        tokenizer.advance();
        compileTerm();
        switch (op) {
            case '+':
                writer.writeArithmetic("add");
                break;
            case '-':
                writer.writeArithmetic("sub");
                break;
            case '*':
                writer.writeCall("Math.multiply", 2);
                break;
            case '/':
                writer.writeCall("Math.divide", 2);
                break;
            case '&':
                writer.writeArithmetic("and");
                break;
            case '|':
                writer.writeArithmetic("or");
                break;
            case '<':
                writer.writeArithmetic("lt");
                break;
            case '>':
                writer.writeArithmetic("gt");
                break;
            case '=':
                writer.writeArithmetic("eq");
                break;
        }
    }
}

void CompilationEngine::compileTerm() {
    // write vm commands that push the evaluated value to the stack
    std::string token;
    switch (tokenizer.tokenType()) {
        case TokenType::INT_CONST:
            writer.writePush("constant", tokenizer.intVal());
            tokenizer.advance();
            break;
        case TokenType::STRING_CONST:
            token = tokenizer.stringVal();
            tokenizer.advance();
            writer.writePush("constant", token.length());
            writer.writeCall("String.new", 1);

            for (char c : token) {
                writer.writePush("constant", c);
                writer.writeCall("String.appendChar", 2);
            }
            break;
        case TokenType::KEYWORD:
            token = tokenizer.keyWord();
            tokenizer.advance();
            if (token == "true") {
                writer.writePush("constant", 0);
                writer.writeArithmetic("not");
            } else if (token == "false" || token == "null") {
                writer.writePush("constant", 0);
            } else if (token == "this") {
                writer.writePush("pointer", 0);
            }
            break;
        case TokenType::IDENTIFIER:
            // varName | varName '[' expression ']' | subroutineCall
            token = tokenizer.identifier();
            tokenizer.advance();

            if (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == '[') {
                // varName '[' expression ']'
                writer.writePush(subroutineSyms.kindOf(token), subroutineSyms.indexOf(token));

                process("[");
                compileExpression();
                process("]");

                writer.writeArithmetic("add");
                writer.writePop("pointer", 1);
                writer.writePush("that", 0);
            } else if (tokenizer.tokenType() == TokenType::SYMBOL && (tokenizer.symbol() == '(' || tokenizer.symbol() == '.')) {
                // subroutineName '(' expressionList ')' | (className | varName) '.' subroutineName '(' expressionList ')'
                compileSubroutineCall(token);
            } else {
                // varName
                std::string kind;
                int index;
                if (subroutineSyms.kindOf(token) != "NONE") {
                    // local variable
                    kind = subroutineSyms.kindOf(token);
                    index = subroutineSyms.indexOf(token);
                } else {
                    // class variable
                    kind = classSyms.kindOf(token);
                    index = classSyms.indexOf(token);
                }
                writer.writePush(kind, index);
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
                tokenizer.advance();
                compileTerm();
                if (tokenizer.symbol() == '-') {
                    writer.writeArithmetic("neg");
                } else if (tokenizer.symbol() == '~') {
                    writer.writeArithmetic("not");
                }
            }
        case TokenType::UNKNOWN:
            break;
    };
}

int CompilationEngine::compileExpressionList() {
    int nArgs = 0;
    // (expression (',' expression)*)?
    if (tokenizer.tokenType() != TokenType::SYMBOL || tokenizer.symbol() != ')') {
        compileExpression();
        nArgs++;
        while (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == ',') {
            // (',' expression)*
            tokenizer.advance();
            compileExpression();
            nArgs++;
        }
    }
    return nArgs;
}

void CompilationEngine::process(std::string expected) {
    std::string actual;
    switch (tokenizer.tokenType()) {
        case TokenType::KEYWORD:
            actual = tokenizer.keyWord();
            break;
        case TokenType::SYMBOL:
            actual = tokenizer.symbol();
            break;
        default:
            throw std::runtime_error("Unexpected token type");
    }
    if (actual != expected) {
        throw std::runtime_error("Expected " + expected + ", but got " + actual);
    }
    tokenizer.advance();
}

std::string CompilationEngine::processType() {
    std::string type;
    // "int" or "char" or "boolean" or className
    type = tokenizer.tokenType() == TokenType::KEYWORD ? tokenizer.keyWord() : tokenizer.identifier();
    tokenizer.advance();
    return type;
}


void CompilationEngine::compileSubroutineCall(std::string token) {
    // subroutineName '(' expressionList ')' | (className | varName) '.' subroutineName '(' expressionList ')'
    // the first token has been consumed to determine the type of the term

    bool isMethod;
    std::string calleeClass, calleeSubroutine;

    // handle the prefix
    if (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == '.') {
        process(".");
        calleeSubroutine = tokenizer.identifier();
        tokenizer.advance();
        // (className | varName) '.' subroutineName
        if (subroutineSyms.kindOf(token) != "NONE") {
            isMethod = true;
            calleeClass = subroutineSyms.typeOf(token);
            writer.writePush(subroutineSyms.kindOf(token), subroutineSyms.indexOf(token));
        } else if (classSyms.kindOf(token) != "NONE") {
            isMethod = true;
            calleeClass = classSyms.typeOf(token);
            writer.writePush(classSyms.kindOf(token), classSyms.indexOf(token));
        } else {
            // className
            isMethod = false;
            calleeClass = token;
        }
    } else {
        // subroutineName
        isMethod = true;
        calleeClass = className;
        calleeSubroutine = token;
        writer.writePush("pointer", 0);
    }

    process("(");
    int nArgs = compileExpressionList();
    if (isMethod) {
        nArgs++;
    }
    process(")");

    writer.writeCall(calleeClass + "." + calleeSubroutine, nArgs);
}
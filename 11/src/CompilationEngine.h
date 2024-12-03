#include <fstream>
#include <unordered_set>
#include "JackTokenizer.h"
#include "SymbolTable.h"
#include "VMWriter.h"

class CompilationEngine {
public:
    CompilationEngine(JackTokenizer &input, std::ofstream &output);
    void compileClass();
    void compileClassVarDec();
    void compileSubroutine();
    void compileParameterList();
    void compileSubroutineBody();
    void compileVarDec();
    void compileStatements();
    void compileLet();
    void compileIf();
    void compileWhile();
    void compileDo();
    void compileReturn();
    void compileExpression();
    void compileTerm();
    int compileExpressionList();

private:
    JackTokenizer &tokenizer;
    VMWriter writer;
    std::string className;
    SymbolTable classSyms = SymbolTable();
    SymbolTable subroutineSyms = SymbolTable();
    std::string subroutineName;
    std::string subroutineType;
    std::unordered_set<std::string> opSet = {
        "+", "-", "*", "/", "&", "|", "<", ">", "="
    };
    int labelCount = 0;
    std::string processType();
    void process(std::string str);
    void compileSubroutineCall();
};
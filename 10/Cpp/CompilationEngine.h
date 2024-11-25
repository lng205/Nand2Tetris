#include <fstream>
#include <unordered_set>
#include "JackTokenizer.h"

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
    std::ofstream &output;
    void process(std::string str);
    void processType();
    std::unordered_set<std::string> opSet = {
        "+", "-", "*", "/", "&", "|", "<", ">", "="
    };
    void compileSubroutineCall();
};
#include <fstream>

class VMWriter {
public:
    VMWriter(std::ofstream& out);

    // push segments: constant, argument, local, static, this, that, pointer, temp
    void writePush(std::string seg, int index);

    // pop segments: argument, local, static, this, that, pointer, temp
    void writePop(std::string seg, int index);

    // arithmetic commands: add, sub, neg, eq, gt, lt, and, or, not
    void writeArithmetic(std::string cmd);

    void writeLabel(std::string label);
    void writeGoto(std::string label);
    void writeIf(std::string label);
    void writeCall(std::string name, int nArgs);
    void writeFunction(std::string name, int nVars);
    void writeReturn();
    void close();

private:
    std::ofstream& out;
};
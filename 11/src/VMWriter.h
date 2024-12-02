#include <fstream>

class VMWriter {
public:
    VMWriter(std::ofstream& out);
    void writePush(std::string seg, int index);
    void writePop(std::string seg, int index);
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
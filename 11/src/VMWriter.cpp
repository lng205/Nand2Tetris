#include "VMWriter.h"

VMWriter::VMWriter(std::ofstream& out) : out(out) {}

void VMWriter::writePush(std::string seg, int index) {
    out << "push " << seg << " " << index << std::endl;
}

void VMWriter::writePop(std::string seg, int index) {
    out << "pop " << seg << " " << index << std::endl;
}

void VMWriter::writeArithmetic(std::string cmd) {
    out << cmd << std::endl;
}

void VMWriter::writeLabel(std::string label) {
    out << "label " << label << std::endl;
}

void VMWriter::writeGoto(std::string label) {
    out << "goto " << label << std::endl;
}

void VMWriter::writeIf(std::string label) {
    out << "if-goto " << label << std::endl;
}

void VMWriter::writeCall(std::string name, int nArgs) {
    out << "call " << name << " " << nArgs << std::endl;
}

void VMWriter::writeFunction(std::string name, int nVars) {
    out << "function " << name << " " << nVars << std::endl;
}

void VMWriter::writeReturn() {
    out << "return" << std::endl;
}

void VMWriter::close() {
    out.close();
}
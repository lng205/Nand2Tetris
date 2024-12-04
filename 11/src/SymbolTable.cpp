#include "SymbolTable.h"

SymbolTable::SymbolTable() {
    count[Kind::STATIC] = 0;
    count[Kind::FIELD] = 0;
    count[Kind::ARG] = 0;
    count[Kind::VAR] = 0;
}

std::string SymbolTable::kindToString(Kind kind) {
    switch (kind) {
        case Kind::STATIC:
            return "static";
        case Kind::FIELD:
            return "this";
        case Kind::ARG:
            return "argument";
        case Kind::VAR:
            return "local";
    }
}

void SymbolTable::reset() {
    table.clear();
    count.clear();
}

void SymbolTable::define(std::string name, std::string type, Kind kind) {
    table[name] = std::make_tuple(type, kind, varCount(kind));
    count[kind]++;
}

int SymbolTable::varCount(Kind kind) {
    return count[kind];
}

std::string SymbolTable::kindOf(std::string name) {
    if (table.find(name) == table.end()) {
        return "NONE";
    }
    return kindToString(std::get<1>(table[name]));
}

std::string SymbolTable::typeOf(std::string name) {
    return std::get<0>(table[name]);
}

int SymbolTable::indexOf(std::string name) {
    return std::get<2>(table[name]);
}
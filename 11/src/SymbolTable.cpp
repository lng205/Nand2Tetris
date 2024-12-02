#include "SymbolTable.h"

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

std::optional<Kind> SymbolTable::kindOf(std::string name) {
    if (table.find(name) == table.end()) {
        return std::nullopt;
    }
    return std::get<1>(table[name]);
}

std::string SymbolTable::typeOf(std::string name) {
    return std::get<0>(table[name]);
}

int SymbolTable::indexOf(std::string name) {
    return std::get<2>(table[name]);
}
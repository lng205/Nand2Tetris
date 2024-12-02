#include <string>
#include <optional>

enum Kind {
    STATIC,
    FIELD,
    ARG,
    VAR
};

class SymbolTable {
public:
    SymbolTable();
    void reset();
    void define(std::string name, std::string type, Kind kind);
    int varCount(Kind kind);
    std::optional<Kind> kindOf(std::string name);
    std::string typeOf(std::string name);
    int indexOf(std::string name);
};
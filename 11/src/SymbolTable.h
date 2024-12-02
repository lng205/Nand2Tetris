#include <string>
#include <optional>
#include <unordered_map>

enum Kind {
    STATIC,
    FIELD,
    ARG,
    VAR
};

class SymbolTable {
public:
    void reset();
    void define(std::string name, std::string type, Kind kind);
    int varCount(Kind kind);
    std::optional<Kind> kindOf(std::string name);
    std::string typeOf(std::string name);
    int indexOf(std::string name);

private:
    // name -> (type, kind, index)
    std::unordered_map<std::string, std::tuple<std::string, Kind, int>> table;
    std::unordered_map<Kind, int> count;
};
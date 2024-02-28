#include "Parser.cpp"

int main() {
    std::string file_name;
    getName(file_name);
    std::ifstream file(file_name);
    AllFramesVectorClass Storage;
    Parse(file, Storage);
    Storage.CoutAll();
}


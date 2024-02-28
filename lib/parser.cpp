#include "parser.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;


//func for deleting tralling spaces from string
void DeleteSpaces(string &my_string, int left_pos, int right_pos) {

    //удаляем пробелы в начале строки
    while (my_string[left_pos] == ' ') {
        left_pos += 1;
    }

    //удаляем пробелы в конце строки
    while (my_string[right_pos] == ' ') {
        right_pos -= 1;
    }

    //очищенная от лишних пробелов строка
    my_string = my_string.substr(left_pos, right_pos - left_pos + 1);
}

/*****************************************************/
/*****************************************************/

//get key value from string func
bool omfl::Parser::GetKeyValueFromString(string &input_str) {
    int begin_of_string = 0;
    int end_of_string = input_str.size() - 1;

    //flags for comment and string mode
    bool comment_mode = false;
    bool string_mode = false;

    //checking mode
    while (end_of_string >= 0) {
        if (input_str[end_of_string] == '"') {
            string_mode = !string_mode;
        }
        if ((input_str[end_of_string] == '#') && !string_mode) {
            comment_mode = true;
            end_of_string -= 1;
            break;
        }
        end_of_string -= 1;
    }
    if (comment_mode == false) {
        end_of_string = input_str.size() - 1;
    }

    //deleting useless spaces
    DeleteSpaces(input_str, begin_of_string, end_of_string);

    //ищем позицию со знаком '='
    int equals_pos = input_str.find('=');
    if (equals_pos == string::npos) {
        // No '=' found
        return false;
    }

    //выделяем key из строки
    string key_from_input_string = input_str.substr(0, equals_pos);
    DeleteSpaces(key_from_input_string, 0, key_from_input_string.size() - 1);
    if (key_from_input_string.empty()) {
        return false;
    }

    //выделяем значение из строки
    string value_from_input_string = input_str.substr(equals_pos + 1);
    DeleteSpaces(value_from_input_string, 0, value_from_input_string.size() - 1);
    if (value_from_input_string.empty()) {
        return false;
    }

    //creating new value
    omfl::Value new_value;

    //присваиваем значения флагам корректности
    if (new_value.setValue(value_from_input_string) && SetKey(key_from_input_string, new_value)) {
        return true;
    } else {
        return false;
    }
}


bool omfl::Parser::ParseSingleString(string &currentSectionName, string &currentString) {
    // Trim leading and trailing spaces
    DeleteSpaces(currentString, 0, currentString.size() - 1);

    if (currentString.empty() || currentString[0] == '#') {
        // Comment line, ignore
        return true;
    } else if (currentString[0] == '[' && currentString[currentString.size() - 1] == ']') {
        // Section header
        currentSectionName = currentString.substr(1, currentString.size() - 2);

        vector<string> names;
        string currentName;
        for (char ch: currentSectionName) {
            if (ch == '.') {
                names.emplace_back(currentName);
                currentName = "";
                continue;
            }
            currentName = currentName + ch;
        }
        names.emplace_back(currentName);

        for (string str: names) {
            if (!IsKeyCorrect(str)) {
                return false;
            }
        }
        return true;
    } else {
        // Key-value pair
        string addingString = currentSectionName;
        if (addingString.empty()) {
            addingString = addingString + currentString;
        } else {
            addingString = addingString + "." + currentString;
        }
        return GetKeyValueFromString(addingString);
    }
}

/*****************************************************/
//parse funcs

//parse for files
omfl::Parser omfl::parse(const filesystem::path &path) {
    ifstream inputFile(path);
    Parser ans;
    bool isValidFlag = true;
    string currentSectionName;
    // Read each line of the file
    string currentString;
    while (getline(inputFile, currentString)) {
        isValidFlag = isValidFlag && ans.ParseSingleString(currentSectionName, currentString);
    }
    // Set the validity of the parser
    ans.setValid(isValidFlag);
    return ans;
}

//get func
omfl::Parser omfl::Parser::Get(const string &key) const {
    int dotPos = key.find('.');
    if (dotPos != string::npos) {
        auto curKey = key.substr(0, dotPos);
        auto it = sections.find(curKey);
        if (it != sections.end()) {
            Parser cur = it->second;
            return cur.Get(key.substr(dotPos + 1));
        } else {
            throw out_of_range("Key not found");
        }
    } else {
        auto it = sections.find(key);
        if (it != sections.end()) {
            return it->second;
        } else {
            throw out_of_range("Key not found");
        }
    }
}

//parse for strings
omfl::Parser omfl::parse(const string &str) {
    Parser answer;
    auto isValidFlag = true;
    istringstream strings(str);
    string currentSectionName;
    while (strings.eof() == false) {
        string currentString;
        getline(strings, currentString);
        isValidFlag = isValidFlag & answer.ParseSingleString(currentSectionName, currentString);
    }
    //setting valid
    answer.setValid(isValidFlag);
    return answer;
}

//valid check func
bool omfl::Parser::valid() const {
    return isValidFlag;
}

/*****************************************************/

//funcs for int
int omfl::Parser::AsInt() {
    return value.getIntValue();
}

bool omfl::Parser::IsInt() {
    return value.getType() == omfl::kINT;
}

int omfl::Parser::AsIntOrDefault(int default_int) {
    return IsInt() ? AsInt() : default_int;
}


int omfl::Value::getIntValue() const {
    return valueIntType;
}

/*****************************************************/

//funcs for float
bool omfl::Parser::IsFloat() {
    return value.getType() == omfl::kFLOAT;
}

float omfl::Parser::AsFloat() {
    return value.getFloatValue();
}

double omfl::Parser::AsFloatOrDefault(double default_double) {
    return IsFloat() ? AsFloat() : default_double;
}


float omfl::Value::getFloatValue() const {
    return valueFloatType;
}

/*****************************************************/

//funcs for string
bool omfl::Parser::IsString() {
    return value.getType() == omfl::kSTRING;
}

string omfl::Parser::AsString() {
    return value.getStringValue();
}

string omfl::Parser::AsStringOrDefault(string default_string) {
    return IsString() ? AsString() : default_string;
}

const string &omfl::Value::getStringValue() const {
    return valueStringType;
}

/*****************************************************/

//funcs for array
bool omfl::Parser::IsArray() {
    return value.getType() == omfl::kARRAY;
}

omfl::Parser omfl::Parser::operator[](int index) {
    if (index < 0 || index >= static_cast<int>(value.getArray().size())) {
        throw out_of_range("index out of range");
    }
    Parser answer;
    answer.value = value.getArray()[index];
    return answer;
}


vector<omfl::Value> &omfl::Value::getArray() {
    return array;
}

/*****************************************************/

//funcs for bool
bool omfl::Parser::IsBool() {
    return value.getType() == omfl::kBOOL;
}

bool omfl::Parser::AsBool() {
    return value.getBoolValue();
}

bool omfl::Value::getBoolValue() const {
    return valueBoolType;
}

/*****************************************************/

//set funcs

//first setkey func
bool omfl::Parser::SetKey(string &key, const omfl::Value &value) {
    return SetKey(key, value, *this);
}

//overloaded setkey func for section
bool omfl::Parser::SetKey(string &key, const omfl::Value &value, omfl::Parser &currentSection) {
    size_t endPosOfSectionName = key.find('.');
    if (endPosOfSectionName != string::npos) {
        string currentName = key.substr(0, endPosOfSectionName);
        string lastName = key.substr(endPosOfSectionName + 1);
        if (currentSection.sections.count(currentName) && currentSection.sections[currentName].is_key_) {
            return false;
        } else {
            omfl::Parser &section = currentSection.sections[currentName];
            return SetKey(lastName, value, section) && IsKeyCorrect(currentName);
        }
    } else {
        if (currentSection.sections.count(key)) {
            return false;
        } else {
            omfl::Parser lastSection;
            lastSection.value = value;
            lastSection.is_key_ = true;
            currentSection.sections[key] = lastSection;
            return IsKeyCorrect(key);
        }
    }
}


//set valid func
void omfl::Parser::setValid(bool valid) {
    isValidFlag = valid;
}

//first set value func
bool omfl::Value::setValue(const string &initialString, omfl::Value &value) {
    string str;
    int begin = 0;
    int end = initialString.size();
    // Trim leading whitespaces
    while (begin < end && initialString[begin] == ' ') {
        begin += 1;
    }
    // Trim trailing whitespaces
    while (end > begin && initialString[end - 1] == ' ') {
        end -= 1;
    }
    // Check if the string is a quoted string
    if (initialString[begin] == '"' && initialString[end - 1] == '"') {
        // Check if the string has any unescaped quotes
        for (int i = begin + 1; i < end - 1; ++i) {
            if (initialString[i] == '"' && initialString[i - 1] != '\\') {
                return false;
            }
            value.valueStringType += initialString[i];
        }
        value.type = kSTRING;
        return true;
    }
        // Check if the string is an array
    else if (initialString[begin] == '[' && initialString[end - 1] == ']') {
        bool isArrayCorrect = true;
        value.type = kARRAY;
        // Split the array elements by ','
        vector<string> currentArr;
        string newInitialString;
        int bracketsDelt = 0;
        bool wasStringSymbolFlag = false;
        for (int i = begin + 1; i < end - 1; ++i) {
            if (initialString[i] == '"') {
                wasStringSymbolFlag = !wasStringSymbolFlag;
            } else if (initialString[i] == '[' && !wasStringSymbolFlag) {
                bracketsDelt += 1;
            } else if (initialString[i] == ']' && !wasStringSymbolFlag) {
                bracketsDelt -= 1;
            } else if (bracketsDelt == 0 && initialString[i] == ',' && !wasStringSymbolFlag) {
                currentArr.emplace_back(newInitialString);
                newInitialString = "";
                continue;
            }
            newInitialString += initialString[i];
        }
        if (bracketsDelt != 0 || wasStringSymbolFlag) {
            return false;
        }
        if (!newInitialString.empty()) {
            currentArr.emplace_back(newInitialString);
        }
        // Recursively parse each element of the array
        for (string &val: currentArr) {
            omfl::Value newVal;
            isArrayCorrect &= setValue(val, newVal);
            value.array.emplace_back(newVal);
        }
        return isArrayCorrect;
    }
        // Check if the string is a boolean
    else if ((end - begin == 4) && initialString.substr(begin, end - begin + 1) == "true") {
        value.type = kBOOL;
        value.valueBoolType = true;
        return true;
    } else if ((end - begin == 5) && initialString.substr(begin, end - begin + 1) == "false") {
        value.type = kBOOL;
        return true;
    }
// Check if the string is a number
    else {
        int countOfDots = 0;
        bool numericMode = true;
        bool integerPart = true;
        bool fractionalPart = true;
        for (int i = begin; i < end; ++i) {
// Check if the string has a leading '+' or '-'
            if ((initialString[i] == '+' || initialString[i] == '-') && (i == begin)) {
                continue;
            }
// Check if the string has a '.' character
            if (initialString[i] == '.') {
                countOfDots += 1;
                if (i == begin ||
                    (i > 0 && (initialString[i - 1] == '+' || initialString[i - 1] == '-'))) {
                    integerPart = false;
                }
                if (i == end - 1) {
                    fractionalPart = false;
                }
            } else {
// Check if the string has non-numeric characters
                numericMode &= initialString[i] <= '9' && initialString[i] >= '0';
            }
        }
// Check if the string is an empty number
        if (begin + 1 == end && (initialString[begin] == '+' || initialString[begin] == '-')) {
            return false;
        }
// Check if the string is an integer
        if (numericMode && countOfDots == 0) {
            value.type = kINT;
            value.valueIntType = stoi(initialString.substr(begin, end - begin));
            return true;
        }
// Check if the string is a float
        if (numericMode && countOfDots == 1 && integerPart && fractionalPart) {
            value.type = kFLOAT;
            value.valueFloatType = stof(initialString.substr(begin, end - begin));
            return true;
        }
    }
    return false;
}


//overloaded set value func
bool omfl::Value::setValue(string &input_str) {
    return setValue(input_str, *this);
}

//get type func
const string &omfl::Value::getType() const {
    return type;
}

//checking key
bool omfl::IsKeyCorrect(const string &key) {
    if (key.empty()) {
        return false;
    }
    // Check if the key only contains alphabetic characters, numbers, hyphens, and underscores
    for (char ch: key) {
        if (!isalnum(ch) && ch != '-' && ch != '_') {
            return false;
        }
    }
    return true;
}
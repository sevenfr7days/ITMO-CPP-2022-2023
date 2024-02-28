#pragma once

#include <filesystem>
#include <istream>
#include <map>
#include <vector>

using namespace std;


namespace omfl {
/*****************************************************/
/*****************************************************/

    //const string names for value type
    const string kINT = "INT";
    const string kFLOAT = "FLOAT";
    const string kBOOL = "BOOL";
    const string kSTRING = "STRING";
    const string kARRAY = "ARRAY";

/*****************************************************/
/*****************************************************/

//key correct checking func
    bool IsKeyCorrect(const string &key);

    // сlass for value
    class Value {
    protected:
        //protected values for class omfl::Value
        string type;
        int valueIntType = 0;
        bool valueBoolType = false;
        float valueFloatType = 0.0;
        string valueStringType = "";
        vector<omfl::Value> array;
    public:
        //set value for strings with value
        bool setValue(const string &initialString, omfl::Value &value);

        //set value just for input_str
        bool setValue(string &input_str);


        //nodiscard keyword means that we can't ignore returning value
        //and we should save that to other value

        //get type func for value
        [[nodiscard]] const string &getType() const;

        //get value for int value func
        [[nodiscard]] int getIntValue() const;

        //get value for float type value func
        [[nodiscard]] float getFloatValue() const;

        //get value for bool type value func
        [[nodiscard]] bool getBoolValue() const;

        //get value for string type value func
        [[nodiscard]] const string &getStringValue() const;


        //vector of omfl::Value for getArray func
        vector<omfl::Value> &getArray();
    };

/*****************************************************/
/*****************************************************/
    //class for parser

    class Parser {
    protected:
        map<string, omfl::Parser> sections;
        bool isValidFlag = true;
        omfl::Value value;

        //mode flags(not used btw)
        bool int_mode = false;
        bool flag_mode = false;
        bool string_mode = false;
        bool array_mode = false;
        bool bool_mode = false;
        bool is_key_ = false;

        /*****************************************************/
        /*****************************************************/

        //setkey func
        bool SetKey(string &key, const omfl::Value &value, omfl::Parser &currentSection);

        //overloaded setkey func
        bool SetKey(string &key, const omfl::Value &value);

    public:

        bool valid() const;

        [[nodiscard]] omfl::Parser Get(const string &key) const;

/*****************************************************/
/*****************************************************/

        //funcs for int values

        bool IsInt();

        int AsInt();

        int AsIntOrDefault(int default_int);

/*****************************************************/
/*****************************************************/
        //funcs for float values

        bool IsFloat();

        float AsFloat();

        double AsFloatOrDefault(double default_double);

/*****************************************************/
/*****************************************************/

        //funcs for bool values
        bool AsBool();

        bool IsBool();

        void setValid(bool valid);

        //funcs for string values

        bool IsString();

        string AsString();

        string AsStringOrDefault(string default_string);

        //funcs for array

        bool IsArray();

        Parser operator[](int index);

        //other funcs

        bool GetKeyValueFromString(string &input_str);

        bool ParseSingleString(string &currentSectionName, string &currentString);

    };

    //parser func
    Parser parse(const filesystem::path &path);

    //overloaded parser func
    Parser parse(const string &str);
}
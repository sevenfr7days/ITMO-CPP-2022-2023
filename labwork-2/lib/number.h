#include <iostream>
#include <vector>
#include <variant>
#include <map>
#include <set>

using data_types = std::variant<bool, int, float, double, std::string, std::nullptr_t>;
enum data_types_index {
  BOOL = 0, INT = 1, FLOAT = 2, DOUBLE = 3, VARCHAR = 4
};


namespace simple_sql {

class Condition {
 public:
  enum ACTION {
    EQ = 0, LESS = 1, GREATER = 2,
    LEQ = 3, GEQ = 4, NEQ = 5,
    ISNULL = 6, IN = 7
  };
  std::string table_name_1;
  size_t column_index_1;

  std::string table_name_2;
  size_t column_index_2;

  ACTION action;
  data_types value;
  std::vector<data_types> values;
  bool reverse = false;
  bool value_comp;

  Condition(std::string& name1, size_t index1,
            std::string& name2, size_t index2,
            std::string& action_symbol, bool r) {
    table_name_1 = name1;
    column_index_1 = index1;
    table_name_2 = name2;
    column_index_2 = index2;
    reverse = r;
    if (action_symbol == "=") action = EQ;
    else if (action_symbol == "<") action = LESS;
    else if (action_symbol == ">") action = GREATER;
    else if (action_symbol == "<=") action = LEQ;
    else if (action_symbol == ">=") action = GEQ;
    else if (action_symbol == "<>" || action_symbol == "!=") action = NEQ;
    else if (action_symbol == "IN") action = IN;
    else if (action_symbol == "IS") action = ISNULL;
    else throw std::runtime_error("Unknown operator in WHERE statement");
  }

  Condition(std::string& name, size_t index,
            std::string& action_symbol, bool r,
            std::vector<data_types> vals) {
    table_name_1 = name;
    column_index_1 = index;
    reverse = r;
    if (action_symbol == "=") action = EQ;
    else if (action_symbol == "<") action = LESS;
    else if (action_symbol == ">") action = GREATER;
    else if (action_symbol == "<=") action = LEQ;
    else if (action_symbol == ">=") action = GEQ;
    else if (action_symbol == "<>" || action_symbol == "!=") action = NEQ;
    else if (action_symbol == "IN") action = IN;
    else if (action_symbol == "IS") action = ISNULL;
    else throw std::runtime_error("Unknown operator in WHERE statement");

    if (action == IN) values = vals;
    else value = vals[0];
  }

  Condition(std::string& val1, std::string& val2,
            std::string& action_symbol) {
    if (action_symbol == "=") value_comp = (val1 == val2);
    else if (action_symbol == "<") value_comp = (val1 < val2);
    else if (action_symbol == ">") value_comp = (val1 > val2);
    else if (action_symbol == "<=") value_comp = (val1 <= val2);
    else if (action_symbol == ">=") value_comp = (val1 >= val2);
    else if (action_symbol == "<>" || action_symbol == "!=") value_comp = (val1 != val2);
    else if (action_symbol == "IN") action = IN;
    else if (action_symbol == "IS") action = ISNULL;
  }

  template <typename T1, typename T2>
  bool check_val(T1 val1, T2 val2) {
    bool result = false;
    if (action == ISNULL) result = (std::is_same_v<T1, std::nullptr_t>);
    else if (action == IN) {
      result = false;
      for (auto& v: values) {
        if (val1 == std::get<T1>(v)) result = true;
      }
    }
    else if constexpr (std::is_same_v<T1, T2>) {
      if (action == EQ) result = (val1 == val2);
      else if (action == LESS) result = (val1 < val2);
      else if (action == GREATER) result = (val1 > val2);
      else if (action == LEQ) result = (val1 <= val2);
      else if (action == GEQ) result = (val1 >= val2);
      else if (action == NEQ) result = (val1 != val2);

    } else if (std::is_same_v<T1, std::nullptr_t> || std::is_same_v<T2, std::nullptr_t>) {
      result = false;
    } else {
      throw std::runtime_error("Unable to compare columns with different types");
    }

    return result;
  }

  bool check(std::map<std::string, std::vector<data_types>>& tname_row) {
    if (table_name_1.empty() && table_name_2.empty()) return value_comp;
    data_types val_row_1 = (table_name_1.empty() ? value : tname_row[table_name_1][column_index_1]);
    data_types val_row_2 = (table_name_2.empty() ? value : tname_row[table_name_2][column_index_2]);

    bool result;
    std::visit([&result, this] (const auto& val1, const auto& val2){
      using T1 = std::decay_t<decltype(val1)>;
      using T2 = std::decay_t<decltype(val2)>;

      if constexpr (std::is_same_v<T1, std::nullptr_t>)
        result = (action == ISNULL);
      else if constexpr (std::is_same_v<T1, bool>)
        result = check_val(val1, val2);
      else if constexpr (std::is_same_v<T1, int>)
        result = check_val(val1, val2);
      else if constexpr (std::is_same_v<T1, float>)
        result = check_val(val1, val2);
      else if constexpr (std::is_same_v<T1, double>)
        result = check_val(val1, val2);
      else if constexpr (std::is_same_v<T1, std::string>) {
        result = check_val(val1, val2);
      }
    }, val_row_1, val_row_2);
    if (reverse) result = !result;
    return result;
  }
};

class Table; // Pre-declaration

class WHERE {
 public:
  enum LOGIC_OPERATORS {AND = -4, OR = -3, LEFT = -2, RIGHT = -1};
  std::vector<Condition> conditions;
  std::vector<int64_t> order;

  WHERE(std::string&,  std::map<std::string, Table>&);

  bool satisfy(std::map<std::string, std::vector<data_types>>&);
};

class Table {

  friend class Database;
  friend class WHERE;

  friend std::ostream& operator<<(std::ostream& os, const simple_sql::Table& table);

 private:
  using data_types = std::variant<bool, int, float, double, std::string, std::nullptr_t>;
  enum data_types_index {
    BOOL = 0, INT = 1, FLOAT = 2, DOUBLE = 3, VARCHAR = 4
  };

  std::string label;
  std::vector<size_t> primary_key_indexes;
  std::map<std::string, std::pair<std::string, std::string>> foreign_keys;
  std::map<size_t, size_t> m_index_varsize;

  std::vector<size_t> column_type;
  std::vector<std::string> column_label;
  std::vector<bool> column_isnull;
  std::vector<std::vector<data_types>> rows;

  std::vector<std::pair<size_t, data_types>> updates;

  void AddUpdates(std::string&, std::string&);

  void Insert(std::map<std::string, std::string>&);

 public:
  Table() = default;
}; // class Table
std::ostream& operator<<(std::ostream&, const Table&);

class Database {
  friend class Table;

 private:
  std::map<std::string, simple_sql::Table> tables;

  void ParseCreateLine(std::vector<std::string>& line, simple_sql::Table& table);

  Table CreateTableRequest(std::stringstream&);

  void UpdateTableRequest(std::stringstream&, Table&, WHERE&);

  void InsertTableRequest(std::stringstream&, Table&);

  void DeleteTableRequest(Table&, WHERE&);

 public:
  Table ParseRequest(const std::string&);

  void SaveToFile(const std::string& path = "Database.txt");

  void LoadFrom(const std::string& path = "Database.txt");
}; // class Database

} // namespace sql


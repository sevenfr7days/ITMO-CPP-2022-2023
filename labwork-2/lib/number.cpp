#include <algorithm>
#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>

#include "number.h"

simple_sql::Table simple_sql::Database::ParseRequest(const std::string& str) {
  std::string s = str;
  std::string delimiter = ";\t\n() ";
  std::replace_if(s.begin(), s.end(), [&](char c) {
    return delimiter.find(c) != std::string::npos; }, ' ');

  size_t pos = 0;
  while ((pos = s.find(',', pos)) != std::string::npos) {
    s.replace(pos, 1, " , ");
    pos += 2;
  }

  std::string lowered_where = str;
  std::string where_conditions = str;
  std::transform(lowered_where.begin(), lowered_where.end(), lowered_where.begin(), ::tolower);
  if (lowered_where.find("where") != std::string::npos)
    where_conditions = where_conditions.substr(lowered_where.find("where") + 6, std::string::npos);
  else
    where_conditions.clear();
  if (where_conditions.ends_with(';')) where_conditions.pop_back();

  std::stringstream ss(s);
  std::string word;
  ss >> word;
  std::transform(word.begin(), word.end(), word.begin(), ::tolower);

  if (word == "create") {
    ss >> word; // skip word "TABLE"
    Table t = CreateTableRequest(ss);
    if (tables.contains(t.label)) throw std::runtime_error("Table " + t.label + " already exists");
    tables[t.label] = t;

    return tables[t.label];

  } else if (word == "drop") {
    ss >> word;
    std::transform(word.begin(), word.end(), word.begin(), ::tolower);
    if (word == "table") {
      ss >> word;
      bool found = tables.erase(word);
      if (!found) throw std::runtime_error("Table " + word + " does not exist in database");
    } else {
      throw std::runtime_error("Unknown command: DROP " + word);
    }
    Table t;
    return t;

  } else if (word == "update") {
    ss >> word; // Read table name
    auto table_iter = tables.find(word);
    if (table_iter == tables.end()) throw std::runtime_error("Table " + word + " does not exist in database");

    ss >> word; // Try to read SET
    std::transform(word.begin(), word.end(), word.begin(), ::toupper);
    if (word != "SET") throw std::runtime_error("Wrong statement: UPDATE <table_name> " + word);


    // PARSE WHERE
    std::map<std::string, Table> related_tables = {{table_iter->first, table_iter->second}};
    WHERE current_where(where_conditions, related_tables);

    // Read conditions
    UpdateTableRequest(ss, table_iter->second, current_where);

    return table_iter->second;

  } else if (word == "insert") {
    ss >> word; // skip INTO
    std::transform(word.begin(), word.end(), word.begin(), ::toupper);
    if (word != "INTO") throw std::runtime_error("Wrong statement: INSERT " + word);

    ss >> word; // Read table name;
    auto table_iter = tables.find(word);
    if (table_iter == tables.end()) throw std::runtime_error("Table " + word + " does not exist in database");

    std::string values = str;
    size_t pos = 0;
    while ((pos = values.find(',', pos)) != std::string::npos) {
      values.replace(pos, 1, " , ");
      pos += 2;
    }
    pos = 0;
    while ((pos = values.find('(', pos)) != std::string::npos) {
      values.replace(pos, 1, " ( ");
      pos += 2;
    }
    pos = 0;
    while ((pos = values.find(')', pos)) != std::string::npos) {
      values.replace(pos, 1, " ) ");
      pos += 2;
    }
    std::stringstream ss_temp(values);
    ss_temp >> word;
    ss_temp >> word;
    ss_temp >> word;

    InsertTableRequest(ss_temp, table_iter->second);

    return table_iter->second;

  } else if (word == "delete") {
    ss >> word; // Read FROM
    ss >> word; // Read table name;
    auto table_iter = tables.find(word);
    if (table_iter == tables.end()) throw std::runtime_error("Table " + word + " does not exist in database");

    // PARSE WHERE
    std::map<std::string, Table> related_tables = {{table_iter->first, table_iter->second}};
    WHERE current_where(where_conditions, related_tables);
    // table_iter->second.ParseWhere(where_conditions);

    DeleteTableRequest(table_iter->second, current_where);

    return table_iter->second;

  } else if (word == "select") {
    std::string select_from = s;
    std::replace_if(select_from.begin(), select_from.end(), [&](char c) { return c == ','; }, ' ');
    std::stringstream ss_skip(select_from);
    ss_skip >> word; // Skip SELECT

    std::vector<std::string> select_column_labels;
    while (ss_skip >> word) {
      std::string check = word;
      std::transform(check.begin(), check.end(), check.begin(), ::toupper);
      if (check == "FROM") break;
      select_column_labels.emplace_back(word);
    }

    std::map<std::string, Table> related_tables;
    std::string left_table_label;
    while (ss_skip >> word) {
      std::string check = word;
      std::transform(check.begin(), check.end(), check.begin(), ::toupper);
      if (check == "WHERE" || check == "JOIN" ||
          check == "RIGHT" || check == "LEFT" || check == "INNER")
        break;

      auto table_iter_1 = tables.find(word);
      if (table_iter_1 == tables.end()) throw std::runtime_error("Table " + word + " does not exist in database");
      left_table_label = table_iter_1->first;
      related_tables[table_iter_1->first] = table_iter_1->second;
      word.clear();
    }

    std::transform(word.begin(), word.end(), word.begin(), ::toupper);

    Table selected;
    enum JOIN_MODE { INNER = 0, RIGHT = 1, LEFT = 2 };
    int cur_mode = INNER;
    if (word == "INNER" || word == "JOIN" ||
        word == "RIGHT" || word == "LEFT") {
      // ParseJoinRequest;
      if (word == "INNER") {
        ss_skip >> word;
      } else if (word == "RIGHT") {
        cur_mode = RIGHT;
        ss_skip >> word;
      } else if (word == "LEFT") {
        cur_mode = LEFT;
        ss_skip >> word;
      }
      std::transform(word.begin(), word.end(), word.begin(), ::toupper);
      if (word != "JOIN") throw std::runtime_error("Expected word JOIN");

      ss_skip >> word;
      auto table_iter_2 = tables.find(word);
      if (table_iter_2 == tables.end()) throw std::runtime_error("Table " + word + " does not exist in database");
      related_tables[table_iter_2->first] = table_iter_2->second;
      std::string right_table_label = table_iter_2->first;

      word.clear();
      ss_skip >> word; // ON
      std::transform(word.begin(), word.end(), word.begin(), ::toupper);
      if (!word.empty() && word != "ON" && word != "WHERE")
        throw std::runtime_error("Expected keyword ON in JOIN statement");

      std::string lowered_on = s;
      std::string on_conditions = s;
      std::transform(lowered_on.begin(), lowered_on.end(), lowered_on.begin(), ::tolower);
      if (lowered_on.find(" on ") != std::string::npos) {
        auto end_point = lowered_on.find(" where ");
        if (end_point != std::string::npos) {
          on_conditions = on_conditions.substr(lowered_on.find(" on ") + 4,
                                               end_point - lowered_on.find(" on ") - 4);
        } else {
          on_conditions = on_conditions.substr(lowered_on.find(" on ") + 4, end_point);
        }
      } else {
        on_conditions.clear();
      }

      WHERE on_statement(on_conditions, related_tables);
      WHERE current_where(where_conditions, related_tables);

      for (auto& column : select_column_labels) {
        if (column == "*") {
          for (size_t i = 0; i < related_tables[left_table_label].column_label.size(); i++) {
            selected.column_type.emplace_back(related_tables[left_table_label].column_type[i]);
            selected.column_label.emplace_back(related_tables[left_table_label].column_label[i]);
          }
          for (size_t i = 0; i < related_tables[right_table_label].column_label.size(); i++) {
            selected.column_type.emplace_back(related_tables[right_table_label].column_type[i]);
            selected.column_label.emplace_back(related_tables[right_table_label].column_label[i]);
          }
          continue;
        }

        std::string table_name;
        std::string column_name;
        if (column.find('.') != std::string::npos) {
          table_name = column.substr(0, column.find('.'));
          column_name = column.substr(column.find('.') + 1, std::string::npos);
        } else {
          column_name = column;
        }

        bool is_found = false;
        for (const auto& kv : related_tables) {
          for (size_t i = 0; i < kv.second.column_label.size(); i++) {
            if (column_name == kv.second.column_label[i]) {
              if (!table_name.empty() && kv.first != table_name) continue;
              if (is_found) throw std::runtime_error("Column " + column + " in WHERE clause is ambiguous");
              selected.column_type.emplace_back(kv.second.column_type[i]);
              selected.column_label.emplace_back(column);
              is_found = true;
              break;
            }
          }
        }
        if (!is_found) throw std::runtime_error("Column " + column + " is unknown in WHERE clause");
      }

      std::map<std::string, std::vector<data_types>> check;
      if (cur_mode == INNER) {
        for (auto& row_left : related_tables[left_table_label].rows) {
          check[left_table_label] = row_left;
          for (auto& row_right : related_tables[right_table_label].rows) {
            check[right_table_label] = row_right;
            if (on_statement.satisfy(check) && current_where.satisfy(check)) {
              std::vector<data_types> new_row;
              for (auto& column : select_column_labels) {
                if (column == "*") {
                  for (auto& elem : row_left)
                    new_row.emplace_back(elem);
                  for (auto& elem : row_right)
                    new_row.emplace_back(elem);
                  continue;
                }

                std::string table_name;
                std::string column_name;
                if (column.find('.') != std::string::npos) {
                  table_name = column.substr(0, column.find('.'));
                  column_name = column.substr(column.find('.') + 1, std::string::npos);
                } else {
                  column_name = column;
                }

                bool is_found = false;
                for (const auto& kv : related_tables) {
                  for (size_t i = 0; i < kv.second.column_label.size(); i++) {
                    if (column_name == kv.second.column_label[i]) {
                      if (!table_name.empty() && kv.first != table_name) continue;
                      if (is_found) throw std::runtime_error("Column " + word + " in WHERE clause is ambiguous");
                      if (kv.first == left_table_label)
                        new_row.emplace_back(row_left[i]);
                      else
                        new_row.emplace_back(row_right[i]);
                      is_found = true;
                      break;
                    }
                  }
                }
                if (!is_found) throw std::runtime_error("Column " + word + " is unknown in WHERE clause");
              }
              selected.rows.emplace_back(new_row);
            }
          }
        }
      } else if (cur_mode == LEFT) {
        for (auto& row_left : related_tables[left_table_label].rows) {
          check[left_table_label] = row_left;
          bool found_match = false;
          for (auto& row_right : related_tables[right_table_label].rows) {
            check[right_table_label] = row_right;
            if (on_statement.satisfy(check) && current_where.satisfy(check)) {
              found_match = true;
              std::vector<data_types> new_row;
              for (auto& column : select_column_labels) {
                if (column == "*") {
                  for (auto& elem : row_left)
                    new_row.emplace_back(elem);
                  for (auto& elem : row_right)
                    new_row.emplace_back(elem);
                  continue;
                }

                std::string table_name;
                std::string column_name;
                if (column.find('.') != std::string::npos) {
                  table_name = column.substr(0, column.find('.'));
                  column_name = column.substr(column.find('.') + 1, std::string::npos);
                } else {
                  column_name = column;
                }

                bool is_found = false;
                for (const auto& kv : related_tables) {
                  for (size_t i = 0; i < kv.second.column_label.size(); i++) {
                    if (column_name == kv.second.column_label[i]) {
                      if (!table_name.empty() && kv.first != table_name) continue;
                      if (is_found) throw std::runtime_error("Column " + word + " in WHERE clause is ambiguous");
                      if (kv.first == left_table_label)
                        new_row.emplace_back(row_left[i]);
                      else
                        new_row.emplace_back(row_right[i]);
                      is_found = true;
                      break;
                    }
                  }
                }
                if (!is_found) throw std::runtime_error("Column " + word + " is unknown in WHERE clause");
              }
              selected.rows.emplace_back(new_row);
            }
          }

          if (!found_match) {
            std::vector<data_types> row_right;
            for (auto elem : related_tables[right_table_label].rows[0]) {
              row_right.emplace_back(nullptr);
            }
            check[right_table_label] = row_right;
            if (current_where.satisfy(check)) {
              std::vector<data_types> new_row;
              for (auto& column : select_column_labels) {
                if (column == "*") {
                  for (auto& elem : row_left)
                    new_row.emplace_back(elem);
                  for (auto& elem : row_right)
                    new_row.emplace_back(elem);
                  continue;
                }

                std::string table_name;
                std::string column_name;
                if (column.find('.') != std::string::npos) {
                  table_name = column.substr(0, column.find('.'));
                  column_name = column.substr(column.find('.') + 1, std::string::npos);
                } else {
                  column_name = column;
                }

                bool is_found = false;
                for (const auto& kv : related_tables) {
                  for (size_t i = 0; i < kv.second.column_label.size(); i++) {
                    if (column_name == kv.second.column_label[i]) {
                      if (!table_name.empty() && kv.first != table_name) continue;
                      if (is_found) throw std::runtime_error("Column " + word + " in WHERE clause is ambiguous");
                      if (kv.first == left_table_label)
                        new_row.emplace_back(row_left[i]);
                      else
                        new_row.emplace_back(row_right[i]);
                      is_found = true;
                      break;
                    }
                  }
                }
                if (!is_found) throw std::runtime_error("Column " + word + " is unknown in WHERE clause");
              }
              selected.rows.emplace_back(new_row);
            }
          }
        }
      } else if (cur_mode == RIGHT) {
        for (auto& row_right : related_tables[right_table_label].rows) {
          check[right_table_label] = row_right;
          bool found_match = false;
          for (auto& row_left : related_tables[left_table_label].rows) {
            check[left_table_label] = row_left;
            if (on_statement.satisfy(check) && current_where.satisfy(check)) {
              found_match = true;
              std::vector<data_types> new_row;
              for (auto& column : select_column_labels) {
                if (column == "*") {
                  for (auto& elem : row_left)
                    new_row.emplace_back(elem);
                  for (auto& elem : row_right)
                    new_row.emplace_back(elem);
                  continue;
                }

                std::string table_name;
                std::string column_name;
                if (column.find('.') != std::string::npos) {
                  table_name = column.substr(0, column.find('.'));
                  column_name = column.substr(column.find('.') + 1, std::string::npos);
                } else {
                  column_name = column;
                }

                bool is_found = false;
                for (const auto& kv : related_tables) {
                  for (size_t i = 0; i < kv.second.column_label.size(); i++) {
                    if (column_name == kv.second.column_label[i]) {
                      if (!table_name.empty() && kv.first != table_name) continue;
                      if (is_found) throw std::runtime_error("Column " + word + " in WHERE clause is ambiguous");
                      if (kv.first == left_table_label)
                        new_row.emplace_back(row_left[i]);
                      else
                        new_row.emplace_back(row_right[i]);
                      is_found = true;
                      break;
                    }
                  }
                }
                if (!is_found) throw std::runtime_error("Column " + word + " is unknown in WHERE clause");
              }
              selected.rows.emplace_back(new_row);
            }
          }

          if (!found_match) {
            std::vector<data_types> row_left;
            for (auto elem : related_tables[right_table_label].rows[0]) {
              row_left.emplace_back(nullptr);
            }
            check[left_table_label] = row_left;
            if (current_where.satisfy(check)) {
              std::vector<data_types> new_row;
              for (auto& column : select_column_labels) {
                if (column == "*") {
                  for (auto& elem : row_left)
                    new_row.emplace_back(elem);
                  for (auto& elem : row_right)
                    new_row.emplace_back(elem);
                  continue;
                }

                std::string table_name;
                std::string column_name;
                if (column.find('.') != std::string::npos) {
                  table_name = column.substr(0, column.find('.'));
                  column_name = column.substr(column.find('.') + 1, std::string::npos);
                } else {
                  column_name = column;
                }

                bool is_found = false;
                for (const auto& kv : related_tables) {
                  for (size_t i = 0; i < kv.second.column_label.size(); i++) {
                    if (column_name == kv.second.column_label[i]) {
                      if (!table_name.empty() && kv.first != table_name) continue;
                      if (is_found) throw std::runtime_error("Column " + word + " in WHERE clause is ambiguous");
                      if (kv.first == left_table_label)
                        new_row.emplace_back(row_left[i]);
                      else
                        new_row.emplace_back(row_right[i]);
                      is_found = true;
                      break;
                    }
                  }
                }
                if (!is_found) throw std::runtime_error("Column " + word + " is unknown in WHERE clause");
              }
              selected.rows.emplace_back(new_row);
            }
          }
        }
      }

    } else {
      WHERE current_where(where_conditions, related_tables);

      for (auto& column : select_column_labels) {
        if (column == "*") {
          for (size_t i = 0; i < related_tables[left_table_label].column_label.size(); i++) {
            selected.column_type.emplace_back(related_tables[left_table_label].column_type[i]);
            selected.column_label.emplace_back(related_tables[left_table_label].column_label[i]);
          }
          continue;
        }

        std::string table_name;
        std::string column_name;
        if (column.find('.') != std::string::npos) {
          table_name = column.substr(0, column.find('.'));
          column_name = column.substr(column.find('.') + 1, std::string::npos);
        } else {
          column_name = column;
        }

        bool is_found = false;
        for (const auto& kv : related_tables) {
          for (size_t i = 0; i < kv.second.column_label.size(); i++) {
            if (column_name == kv.second.column_label[i]) {
              if (!table_name.empty() && kv.first != table_name) continue;
              if (is_found) throw std::runtime_error("Column " + column + " in WHERE clause is ambiguous");
              selected.column_type.emplace_back(kv.second.column_type[i]);
              selected.column_label.emplace_back(column);
              is_found = true;
              break;
            }
          }
        }
        if (!is_found) throw std::runtime_error("Column " + column + " is unknown in WHERE clause");
      }

      std::map<std::string, std::vector<data_types>> check;
      for (auto& row : related_tables[left_table_label].rows) {
        check[left_table_label] = row;
        if (current_where.satisfy(check)) {
          std::vector<data_types> new_row;
          for (auto& column : select_column_labels) {
            if (column == "*") {
              for (auto& elem : row)
                new_row.emplace_back(elem);
              continue;
            }

            std::string table_name;
            std::string column_name;
            if (column.find('.') != std::string::npos) {
              table_name = column.substr(0, column.find('.'));
              column_name = column.substr(column.find('.') + 1, std::string::npos);
            } else {
              column_name = column;
            }

            bool is_found = false;
            for (const auto& kv : related_tables) {
              for (size_t i = 0; i < kv.second.column_label.size(); i++) {
                if (column_name == kv.second.column_label[i]) {
                  if (!table_name.empty() && kv.first != table_name) continue;
                  if (is_found) throw std::runtime_error("Column " + word + " in WHERE clause is ambiguous");
                  new_row.emplace_back(row[i]);
                  is_found = true;
                  break;
                }
              }
            }
            if (!is_found) throw std::runtime_error("Column " + word + " is unknown in WHERE clause");
          }
          selected.rows.emplace_back(new_row);
        }
      }

    }

    return selected;
  }
  throw std::runtime_error("Unknown command: " + word);
}

data_types getValue(size_t type, std::string& str_value){
  std::string lower_case_value_string = str_value;
  std::transform(lower_case_value_string.begin(), lower_case_value_string.end(),
                 lower_case_value_string.begin(), ::tolower);

  if(lower_case_value_string == "null" || lower_case_value_string.empty()){
    return nullptr;
  }

  data_types value;
  switch (type){
    case BOOL:
      if(lower_case_value_string == "true"){
        value = true;
        break;
      }
      else{
        value = false;
        break;
      }
    case INT:
      value = std::stoi(lower_case_value_string);
      break;

    case FLOAT:
      value = std::stof(lower_case_value_string);
      break;
    case DOUBLE:
      value = std::stod(lower_case_value_string);
      break;

    case VARCHAR:
      std::string varchar_value = str_value.substr(1, str_value.size() - 2);
      value = varchar_value;
      break;
  }
  return value;
}


std::string to_string_function(const data_types& value) {
  std::string text;
  std::visit([&text](const auto& val) {
    using T = std::decay_t<decltype(val)>;
    if constexpr (std::is_same_v<T, std::nullptr_t>)
      text = "NULL";
    else if constexpr (std::is_same_v<T, bool>)
      text = (val ? "true" : "false");
    else if constexpr (std::is_same_v<T, int>)
      text = std::to_string(val);
    else if constexpr (std::is_same_v<T, float>)
      text = std::to_string(val);
    else if constexpr (std::is_same_v<T, double>)
      text = std::to_string(val);
    else if constexpr (std::is_same_v<T, std::string>) {
      text = "\"";
      text += val;
      text += "\"";
    }
  }, value);
  return text;
}

// CREATE
void simple_sql::Database::ParseCreateLine(std::vector<std::string>& line, simple_sql::Table& table) {
  std::string check_word = line[0];
  std::transform(check_word.begin(), check_word.end(), check_word.begin(), ::tolower);

  // Проверка, является ли первое слово "primary"
  if (check_word == "primary") {
    // Устанавливаем PRIMARY KEY
    for (size_t i = 2; i < line.size(); i++) {
      for (size_t j = 0; j < table.column_label.size(); j++) {
        if (table.column_label[j] == line[i]) {
          table.primary_key_indexes.emplace_back(j);
          break;
        }
      }
    }
    return;
  }

  // Проверка, является ли первое слово "foreign"
  if (check_word == "foreign") {
    // Устанавливаем FOREIGN KEY
    check_word = line[1];
    std::transform(check_word.begin(), check_word.end(), check_word.begin(), ::toupper);
    if (check_word != "KEY") throw std::runtime_error("Неизвестная команда FOREIGN " + check_word);
    check_word = line[3];
    std::transform(check_word.begin(), check_word.end(), check_word.begin(), ::toupper);
    if (check_word != "REFERENCES") throw std::runtime_error("Неизвестная команда FOREIGN KEY " + check_word);

    table.foreign_keys[line[2]] = std::make_pair(line[4], line[5]);
    return;
  }

  // Добавление новой колонки
  table.column_label.emplace_back(line[0]);
  check_word = line[1];
  std::transform(check_word.begin(), check_word.end(), check_word.begin(), ::tolower);
  if (check_word == "bool") {
    table.column_type.emplace_back(table.BOOL);
  } else if (check_word == "int") {
    table.column_type.emplace_back(table.INT);
  } else if (check_word == "float") {
    table.column_type.emplace_back(table.FLOAT);
  } else if (check_word == "double") {
    table.column_type.emplace_back(table.DOUBLE);
  } else if (check_word == "varchar") {
    table.column_type.emplace_back(table.VARCHAR);
    table.m_index_varsize.insert({table.column_type.size() - 1, std::stoi(line[2], nullptr, 10)});
  }
  table.column_isnull.emplace_back(true);

  // Добавление ограничений
  size_t max_size = 2 + (table.column_type.back() == table.VARCHAR);
  if (line.size() > max_size) {
    for (size_t i = max_size; i < line.size(); i++) {
      check_word = line[i];
      std::transform(check_word.begin(), check_word.end(), check_word.begin(), ::tolower);
      if (check_word == "primary") {
        // Устанавливаем PRIMARY KEY для текущей колонки
        if (table.primary_key_indexes.empty() ||
            table.primary_key_indexes.back() != (table.column_label.size() - 1)) {
          table.primary_key_indexes.emplace_back(table.column_label.size() - 1);
        }
        i++;
      } else if (check_word == "foreign") {
        // Устанавливаем FOREIGN KEY
        std::string foreign_column_name = line[0];
        check_word = line[i + 1];
        std::transform(check_word.begin(), check_word.end(), check_word.begin(), ::toupper);
        if (check_word != "KEY") throw std::runtime_error("Неизвестная команда FOREIGN " + check_word);
        check_word = line[i + 2];
        std::transform(check_word.begin(), check_word.end(), check_word.begin(), ::toupper);
        if (check_word != "REFERENCES") throw std::runtime_error("Неизвестная команда FOREIGN KEY " + check_word);
        i += 3; // FOREIGN KEY REFERENCES
        table.foreign_keys[foreign_column_name] = std::make_pair(line[i], line[i + 1]);
      } else if (check_word == "not") {
        // Устанавливаем NOT NULL для текущей колонки
        table.column_isnull.back() = false;
        i++;
      }
    }
  }
}


simple_sql::Table simple_sql::Database::CreateTableRequest(std::stringstream& ss) {
  Table table;
  ss >> table.label;

  bool last_word_here = false;
  bool primary_key = false;
  std::vector<std::string> word_line;
  std::string word;
  while (ss >> word) {
    if (word == "," || word.ends_with(',')) {
      word.erase(word.end() - 1); // Удаляем запятую из слова (если есть)
      if (!word.empty()) word_line.emplace_back(word); // Если слово не пустое, добавляем его в вектор word_line
      ParseCreateLine(word_line, table); // Парсим word_line и обновляем объект table
      word_line.clear();
    } else {
      word_line.emplace_back(word);
    }
  }
  if (!word_line.empty()) {
    ParseCreateLine(word_line, table); // Парсим оставшиеся слова в word_line и обновляем объект table
  }
  return table; // Возвращаем объект table, представляющий определение структуры таблицы SQL
}


// WHERE
simple_sql::WHERE::WHERE(std::string& input,
                         std::map<std::string, simple_sql::Table>& related_tables) {
  std::string line;
  bool sign = false;
  for (char c : input) {
    if (c == '\n' || c == '\t' || c == ';') line += ' ';
    else if ((c == '=' || c == '<' || c == '>') && !sign) {
      line += ' ';
      line += c;
      sign = true;
    } else if ((c == '=' || c == '<' || c == '>') && sign) {
      line += c;
      line += ' ';
      sign = false;
    } else if (c == ',' || c == '(' || c == ')') {
      line += ' ';
      line += c;
      line += ' ';
    } else if (sign) {
      line += ' ';
      line += c;
      sign = false;
    } else {
      line += c;
    }
  }

  std::stringstream ss(line);

  size_t step = 0;
  //std::string name;
  std::string table_name_1;
  std::string column_label_1;

  std::string table_name_2;
  std::string column_label_2;

  std::string action;
  bool reverse = false;
  bool many_values = false;
  std::string value_left;
  std::string value_right;
  std::vector<data_types> values;
  bool is_string_value = false;

  std::string word;
  while (ss >> word) {
    if (word == ",") continue;
    if (word == "(" && action == "IN") {
      many_values = true;
      continue;
    } else if (word == ")" && action == "IN") {
      step++;
    }
    std::string check = word;
    std::transform(check.begin(), check.end(), check.begin(), ::toupper);
    if (check == "NOT") {
      reverse = !reverse;
      continue;
    } else if (check == "AND") {
      order.emplace_back(AND);
      continue;
    } else if (check == "OR") {
      order.emplace_back(OR);
      continue;
    } else if (check == "(") {
      order.emplace_back(LEFT);
      continue;
    } else if (check == ")" && step != 3) {
      order.emplace_back(RIGHT);
      continue;
    }

    if (step == 0) {
      if (!word.starts_with('\"') && word.find('.') != std::string::npos && !is_string_value) {

        std::regex double_checker("^(-?)(0|([1-9][0-9]*))(\\.[0-9]+)?$");
        if (std::regex_match(word, double_checker)) {
          value_left = word;
        } else {
          table_name_1 = word.substr(0, word.find('.'));
          column_label_1 = word.substr(word.find('.') + 1, std::string::npos);
        }

      } else if ((word.starts_with('\"') || is_string_value) && !word.ends_with('\"')) {
        is_string_value = true;
        value_left += word + ' ';
        step--;
      } else if (word.ends_with('\"')) {
        is_string_value = false;
        value_left += word;
      } else {
        //value_left = word;
        std::string check_bool = word;
        std::transform(check_bool.begin(), check_bool.end(),
                       check_bool.begin(), ::tolower);
        if (check_bool == "true" || check_bool == "false") {
          value_left = check_bool;
        } else {

          std::regex double_checker("^(-?)(0|([1-9][0-9]*))(\\.[0-9]+)?$");
          if (std::regex_match(word, double_checker)) {
            value_left = word;
          } else {
            bool is_found = false;
            for (const auto& kv : related_tables) {
              for (const auto& column_label : kv.second.column_label) {
                if (word == column_label) {
                  if (is_found) throw std::runtime_error("Column " + word + " in WHERE clause is ambiguous");
                  table_name_1 = kv.first;
                  column_label_1 = column_label;
                  is_found = true;
                  break;
                }
              }
            }
            if (!is_found) throw std::runtime_error("Column " + word + " is unknown in WHERE clause");
          }
        }
      }

      step++;
      continue;
    } else if (step == 1) {
      action += check;
    } else if (step == 2) {
      if (!word.starts_with('\"') && word.find('.') != std::string::npos && !is_string_value) {

        std::regex double_checker("^(-?)(0|([1-9][0-9]*))(\\.[0-9]+)?$");
        if (std::regex_match(word, double_checker)) {
          value_right = word;
        } else {
          table_name_2 = word.substr(0, word.find('.'));
          column_label_2 = word.substr(word.find('.') + 1, std::string::npos);
        }

      } else if ((word.starts_with('\"') || is_string_value) && !word.ends_with('\"')) {
        is_string_value = true;
        value_right += word + ' ';
      } else if (word.ends_with('\"')) {
        is_string_value = false;
        value_right += word;
      } else {
        //value_right = word;
        std::string check_bool = word;
        std::transform(check_bool.begin(), check_bool.end(),
                       check_bool.begin(), ::tolower);
        if (check_bool == "true" || check_bool == "false" || check_bool == "null") {
          value_right = check_bool;
        } else {
          std::regex double_checker("^(-?)(0|([1-9][0-9]*))(\\.[0-9]+)?$");
          if (std::regex_match(word, double_checker)) {
            value_right = word;
          } else {
            bool is_found = false;
            for (const auto& kv : related_tables) {
              for (const auto& column_label : kv.second.column_label) {
                if (word == column_label) {
                  if (is_found) throw std::runtime_error("Column " + word + " in WHERE clause is ambiguous");
                  table_name_2 = kv.first;
                  column_label_2 = column_label;
                  is_found = true;
                  break;
                }
              }
            }
            if (!is_found) throw std::runtime_error("Column " + word + " is unknown in WHERE clause");
          }
        }
      }

      if (!value_left.empty() && !table_name_2.empty()) {
        table_name_1 = table_name_2;
        column_label_1 = column_label_2;
        value_right = value_left;
        if (action == ">") action = "<";
        else if (action == "<") action = ">";
        else if (action == "<=") action = ">=";
        else if (action == ">=") action = "<=";
        value_left.clear();
      }

      if (value_right.ends_with('\"') || (!value_right.starts_with('\"') && !value_right.empty())) {
        if (!table_name_1.empty()) {
          bool has_table = false;
          for (auto& kv : related_tables) {
            if (kv.first == table_name_1) {
              has_table = true;
              break;
            }
          }
          if (!has_table)
            throw std::runtime_error("Unknown " + table_name_1 + "." + column_label_1 + " in WHERE clause");

          values.emplace_back(
              getValue(
                  related_tables[table_name_1].column_type[
                      std::find(
                          related_tables[table_name_1].column_label.begin(),
                          related_tables[table_name_1].column_label.end(),
                          column_label_1
                      ) - related_tables[table_name_1].column_label.begin()
                  ], value_right)
          );
          value_right.clear();
        }
      }

    }

    if (step == 1) step++;
    else if (!many_values && step == 2) step++;

    if (step == 3) {
      // CREATE CONDITION;
      if (values.empty()) {
        if (value_left.empty() && value_right.empty()) {
          bool has_table = false;
          for (auto& kv : related_tables) {
            if (kv.first == table_name_1) {
              has_table = true;
              break;
            }
          }
          if (!has_table)
            throw std::runtime_error("Unknown " + table_name_1 + "." + column_label_1 + " in WHERE clause");

          has_table = false;
          for (auto& kv : related_tables) {
            if (kv.first == table_name_2) {
              has_table = true;
              break;
            }
          }
          if (!has_table)
            throw std::runtime_error("Unknown " + table_name_2 + "." + column_label_2 + " in WHERE clause");
        }

        if (value_left.empty() && value_right.empty()) {
          size_t index_1 = std::find(related_tables[table_name_1].column_label.begin(),
                                     related_tables[table_name_1].column_label.end(),
                                     column_label_1) - related_tables[table_name_1].column_label.begin();
          size_t index_2 = std::find(related_tables[table_name_2].column_label.begin(),
                                     related_tables[table_name_2].column_label.end(),
                                     column_label_2) - related_tables[table_name_2].column_label.begin();

          Condition condition(table_name_1, index_1,
                              table_name_2, index_2,
                              action, reverse);
          order.emplace_back(conditions.size());
          conditions.emplace_back(condition);
        } else {
          Condition condition(value_left, value_right, action);
          order.emplace_back(conditions.size());
          conditions.emplace_back(condition);
        }
      } else {
        size_t index_1 = std::find(related_tables[table_name_1].column_label.begin(),
                                   related_tables[table_name_1].column_label.end(),
                                   column_label_1) - related_tables[table_name_1].column_label.begin();
        Condition condition(table_name_1, index_1,
                            action, reverse,
                            values);
        order.emplace_back(conditions.size());
        conditions.emplace_back(condition);
      }

      table_name_1.clear();
      table_name_2.clear();
      column_label_1.clear();
      column_label_2.clear();
      value_left.clear();
      value_right.clear();
      values.clear();
      action.clear();
      reverse = false;
      many_values = false;
      step = 0;
    }
  }
  if (step == 3) {
    // CREATE CONDITION;
    if (values.empty()) {
      if (value_left.empty() && value_right.empty()) {
        size_t index_1 = std::find(related_tables[table_name_1].column_label.begin(),
                                   related_tables[table_name_1].column_label.end(),
                                   column_label_1) - related_tables[table_name_1].column_label.begin();
        size_t index_2 = std::find(related_tables[table_name_2].column_label.begin(),
                                   related_tables[table_name_2].column_label.end(),
                                   column_label_2) - related_tables[table_name_2].column_label.begin();

        Condition condition(table_name_1, index_1,
                            table_name_2, index_2,
                            action, reverse);
        order.emplace_back(conditions.size());
        conditions.emplace_back(condition);
      } else {
        Condition condition(value_left, value_right, action);
        order.emplace_back(conditions.size());
        conditions.emplace_back(condition);
      }
    } else {
      size_t index_1 = std::find(related_tables[table_name_1].column_label.begin(),
                                 related_tables[table_name_1].column_label.end(),
                                 column_label_1) - related_tables[table_name_1].column_label.begin();
      Condition condition(table_name_1, index_1,
                          action, reverse,
                          values);
      order.emplace_back(conditions.size());
      conditions.emplace_back(condition);
    }

    table_name_1.clear();
    table_name_2.clear();
    column_label_1.clear();
    column_label_2.clear();
    value_left.clear();
    value_right.clear();
    values.clear();
    action.clear();
    reverse = false;
    many_values = false;
    step = 0;
  }
}

// UPDATE
void simple_sql::Table::AddUpdates(std::string& col_label, std::string& new_value) {
  size_t index = 0;
  for (; index < this->column_label.size(); index++) {
    if (col_label == column_label[index]) break;
  }
  if (index == column_label.size()) throw std::runtime_error("No column label " + col_label + " in table " + label);

  size_t type = column_type[index];
  data_types value = getValue(type, new_value);

  updates.emplace_back(std::make_pair(index, value));
}

void simple_sql::Database::UpdateTableRequest(std::stringstream& ss,
                                              simple_sql::Table& table,
                                              simple_sql::WHERE& current_where) {
  bool is_label = true;
  char c;
  std::deque<char> is_where;
  std::string column_label;
  std::string new_value;
  while (ss >> c) {
    if (c == ',') {
      table.AddUpdates(column_label, new_value);

      column_label.clear();
      new_value.clear();
      is_label = true;
      continue;
    }
    if (is_label && c == '=') {
      is_label = false;
      continue;
    }
    if (c == ' ' && (is_label || new_value.empty())) {
      continue;
    }

    if (is_label) column_label += c;
    else new_value += c;

    if (is_where.size() == 5) is_where.pop_front();
    is_where.emplace_back(tolower(c));
    if (is_where[0] == 'w' &&
        is_where[1] == 'h' &&
        is_where[2] == 'e' &&
        is_where[3] == 'r' &&
        is_where[4] == 'e') {
      new_value.erase(new_value.end() - 5, new_value.end());
      break;
    }
  }
  if (!column_label.empty() && !new_value.empty()) {
    if (column_label.find('.') != std::string::npos) {
      std::string table_name = column_label.substr(0, column_label.find('.'));
      column_label = column_label.substr(column_label.find('.') + 1, std::string::npos);

      if (table_name != table.label)
        throw std::runtime_error("Unknown column " + table_name + "." +
            column_label + " in table " + table.label);
    }
    table.AddUpdates(column_label, new_value);
  }

  // Proceed Updates
  std::map<std::string, std::vector<data_types>> tname_row;
  for (auto& row : table.rows) {
    tname_row[table.label] = row;
    if (current_where.satisfy(tname_row)) {
      // Do the update
      for (auto& upd : table.updates) {
        row[upd.first] = upd.second;
      }
    }
  }
  for (size_t i = 0; i < table.rows.size(); i++) {
    std::vector<data_types> primary_i;
    for (auto primary_index : table.primary_key_indexes) {
      primary_i.emplace_back(table.rows[i][primary_index]);
    }
    for (size_t j = i + 1; j < table.rows.size(); j++) {
      std::vector<data_types> primary_j;
      for (auto primary_index : table.primary_key_indexes) {
        primary_j.emplace_back(table.rows[j][primary_index]);
      }
      if (primary_i == primary_j)
        throw std::runtime_error("Update caused repeated PRIMARY KEY in table " + table.label);

    }
  }
  table.updates.clear();
}


// INSERT
void simple_sql::Table::Insert(std::map<std::string, std::string>& values) {
  std::vector<data_types> row;

  size_t type;
  std::string column_name;
  std::string str_value;
  data_types value;

  for (size_t i = 0; i < column_label.size(); i++) {
    type = column_type[i];
    column_name = column_label[i];
    str_value = values[column_name];
    value = getValue(type, str_value);

    if (std::holds_alternative<std::nullptr_t>(value) && !column_isnull[i]) {
      throw std::runtime_error("Column " + column_name + " in " + label + " cannot be NULL");
    }
    row.emplace_back(value);
  }

  std::vector<data_types> primary_insert;
  for (auto primary_index : primary_key_indexes) {
    primary_insert.emplace_back(row[primary_index]);
  }

  for (auto& check_row : rows) {
    std::vector<data_types> primary;
    for (auto primary_index : primary_key_indexes) {
      primary.emplace_back(check_row[primary_index]);
    }
    if (primary == primary_insert) throw std::runtime_error("Inserting repeated PRIMARY KEY in table " + label);
  }
  rows.emplace_back(row);
}


void simple_sql::Database::InsertTableRequest(std::stringstream& ss, simple_sql::Table& table) {
  // Read Column names
  std::vector<std::string> column_order;
  std::map<std::string, std::string> values;

  // Initialize all columns with NULL
  for (auto& col_name : table.column_label) {
    values[col_name] = "";
  }

  std::string column_name;
  while (ss >> column_name) {
    if (column_name == "," ||
        column_name == ")" ||
        column_name == "(")
      continue;
    if (column_name.ends_with(',')) column_name.pop_back();

    std::string word = column_name;
    std::transform(word.begin(), word.end(), word.begin(), ::toupper);
    if (word == "VALUES") break;

    column_order.emplace_back(column_name);
  }

  if (column_order.empty()) {
    for (auto& col_name : table.column_label) {
      column_order.emplace_back(col_name);
    }
  }


  // Read Values
  size_t it = 0;
  std::string value;
  while (ss >> value) {
    if (value == "," || value == "(") continue;
    if (value.ends_with(',')) value.pop_back();
    if (value == ")") {
      if (it != column_order.size()) throw std::runtime_error("Not enough VALUES in INSERT statement");
      else table.Insert(values);
      values.clear();
      it = 0;
      continue;
    }

    values[column_order[it++]] = value;
  }

}


// DELETE
void simple_sql::Database::DeleteTableRequest(simple_sql::Table& table, simple_sql::WHERE& current_where) {
  std::vector<std::vector<data_types>> new_rows;
  std::map<std::string, std::vector<data_types>> tname_row;
  for (auto& row : table.rows) {
    tname_row[table.label] = row;
    if (current_where.satisfy(tname_row)) continue;
    new_rows.emplace_back(row);
  }
  table.rows = new_rows;
}


// Standard output
std::string CenterText(const std::string& input, size_t width) {
  std::string text;
  size_t text_width = input.length();
  size_t left = (width - text_width) / 2;
  size_t right = width - text_width - left;
  if (left <= 0) {
    text = input;
    text.resize(width);
  } else {
    while (left-- > 0) text += ' ';
    text += input;
    while (right-- > 0) text += ' ';
  }
  return text;
}


std::ostream& simple_sql::operator<<(std::ostream& os, const simple_sql::Table& table) {
  std::string split_line =
      "+---------------------------------------------------------------------------------------------------------------------------+";
  size_t width = split_line.size();
  size_t column_num = table.column_label.size();
  std::vector<size_t> column_width;
  for (size_t i = 1; i < column_num; i++) {
    split_line[i * width / column_num] = '+';
    column_width.emplace_back((i * width / column_num) - ((i - 1) * width / column_num) - 1);
  }
  column_width.emplace_back(width - 1 - ((column_num - 1) * width / column_num) - 1);

  // Labels
  os << split_line << '\n';
  std::string row_line = "|";
  for (size_t i = 0; i < column_num; i++) {
    row_line += CenterText(table.column_label[i], column_width[i]) + '|';
  }
  os << row_line << '\n';

  // Rows
  for (auto& row : table.rows) {
    os << split_line << '\n';
    row_line = "|";
    for (size_t i = 0; i < column_num; i++) {
      std::string text = to_string_function(row[i]);

      if (row[i].index() == VARCHAR) {
        text.erase(text.begin());
        text.pop_back();
      }

      row_line += CenterText(text, column_width[i]) + '|';
    }
    os << row_line << '\n';
  }
  os << split_line << '\n';

  return os;
}


// FILE save and load
void simple_sql::Database::SaveToFile(const std::string& path) {
  std::ofstream file(path);
  for (auto& [key, table] : tables) {
    file << table.label << '\n';

    for (auto elem : table.primary_key_indexes) {
      file << elem << ' ';
    }
    file << '\n';

    for (const auto& elem : table.foreign_keys) {
      file << elem.first << ' ' << elem.second.first << ' ' << elem.second.second << ' ';
    }
    file << '\n';

    for (auto kv : table.m_index_varsize) {
      file << kv.first << ' ' << kv.second << ' ';
    }
    file << '\n';

    for (auto elem : table.column_type) {
      file << elem << ' ';
    }
    file << '\n';

    for (const auto& elem : table.column_label) {
      file << elem << ' ';
    }
    file << '\n';

    for (auto elem : table.column_isnull) {
      file << elem << ' ';
    }
    file << '\n';

    for (auto& row : table.rows) {
      for (auto& elem : row) {
        std::string text;
        text = to_string_function(elem);
        file << text << ' ';
      }
      file << '\n';
    }
    file << '\n'; // Empty line to split tables
  }
}


void simple_sql::Database::LoadFrom(const std::string& path) {
  std::ifstream file(path);
  if (file.fail()) {
    throw std::runtime_error("Cannot find/open file: " + path);
  }

  std::string word_line;
  while (std::getline(file, word_line)) {
    Table table;
    table.label = word_line;

    std::getline(file, word_line);
    std::stringstream ss(word_line);
    std::string word;
    while (ss >> word) {
      table.primary_key_indexes.emplace_back(std::stoull(word));
    }

    std::getline(file, word_line);
    ss = std::stringstream(word_line);
    while (ss >> word) {
      std::string table_reference;
      std::string column_reference;
      ss >> table_reference;
      ss >> column_reference;
      table.foreign_keys[word] = std::make_pair(table_reference, column_reference);
    }

    std::getline(file, word_line);
    ss = std::stringstream(word_line);
    std::string value;
    while (ss >> word) {
      ss >> value;
      table.m_index_varsize[std::stoull(word)] = std::stoull(value);
    }

    std::getline(file, word_line);
    ss = std::stringstream(word_line);
    while (ss >> word) {
      table.column_type.emplace_back(std::stoull(word));
    }

    std::getline(file, word_line);
    ss = std::stringstream(word_line);
    while (ss >> word) {
      table.column_label.emplace_back(word);
    }

    std::getline(file, word_line);
    ss = std::stringstream(word_line);
    while (ss >> word) {
      table.column_isnull.emplace_back(word == "1");
    }

    std::map<std::string, std::string> values;
    std::getline(file, word_line);
    while (!word_line.empty()) {
      size_t i = 0;
      ss = std::stringstream(word_line);
      while (ss >> word) {
        values[table.column_label[i++]] = word;
      }
      table.Insert(values);

      std::getline(file, word_line);
    }

    tables[table.label] = table;
  }
}


// Helper functions
bool solve(simple_sql::WHERE& where,
           std::map<std::string, std::vector<data_types>>& tname_row,
           std::vector<int64_t>& order) {
  std::vector<int64_t> or_line;

  bool is_last_and = true;
  bool result = true;
  int64_t action = where.AND;

  for (size_t i = 0; i < order.size(); i++) {
    if (order[i] == where.LEFT) {
      size_t count_left = 0;
      std::vector<int64_t> new_order;
      for (++i; i < order.size(); i++) {
        if (order[i] == where.LEFT) count_left++;
        else if (order[i] == where.RIGHT && count_left > 0) count_left--;
        else if (order[i] == where.RIGHT) break;
        new_order.emplace_back(order[i]);
      }

      if (action == where.OR) {
        or_line.emplace_back(result);
        result = true;
        action = where.AND;
        is_last_and = false;
      }
      if (action == where.AND) {
        result = result && solve(where, tname_row, new_order);
        is_last_and = true;
      }

    } else if (order[i] < 0) {
      action = order[i];
    } else {
      if (action == where.OR) {
        or_line.emplace_back(result);
        result = true;
        action = where.AND;
        is_last_and = false;
      }
      if (action == where.AND) {
        result = result && where.conditions[order[i]].check(tname_row);
        is_last_and = true;
      }
    }
  }
  if (is_last_and) or_line.emplace_back(result);

  bool answer = false;
  for (auto state : or_line) {
    answer = answer || state;
  }

  return answer;
}

bool simple_sql::WHERE::satisfy(std::map<std::string, std::vector<data_types>>& tname_row) {
  return solve(*this, tname_row, this->order);
}
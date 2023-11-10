#include <algorithm>
#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>

#include "number.h"

data_types get_value(size_t type, std::string& str_value){
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
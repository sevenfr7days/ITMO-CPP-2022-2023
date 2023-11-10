#include <gtest/gtest.h>
#include <gtest/gtest_prod.h>

#define protected public
#define private   public

#include "lib/number.h"

#undef protected
#undef private

namespace sql = simple_sql;

TEST(SimpleSQL_Tests, CreateTest1) {
  std::string create =
      "CREATE TABLE cats (\n"
      "  id INT NOT NULL PRIMARY KEY,\n"
      "  name VARCHAR(20) NOT NULL,\n"
      "  info VARCHAR(30)\n"
      ");";

  sql::Database db;
  sql::Table created_table = db.ParseRequest(create);


  ASSERT_TRUE(db.tables.begin()->first == "cats");
  ASSERT_TRUE(created_table.label == "cats");
  ASSERT_TRUE(created_table.rows.empty());

  ASSERT_TRUE(created_table.primary_key_indexes.size() == 1);
  ASSERT_TRUE(created_table.primary_key_indexes[0] == 0);

  ASSERT_TRUE(created_table.m_index_varsize.size() == 2);
  ASSERT_TRUE(created_table.m_index_varsize[1] == 20);
  ASSERT_TRUE(created_table.m_index_varsize[2] == 30);

  ASSERT_TRUE(created_table.column_isnull.size() == 3);
  ASSERT_TRUE(created_table.column_isnull[0] == false);
  ASSERT_TRUE(created_table.column_isnull[1] == false);
  ASSERT_TRUE(created_table.column_isnull[2] == true);
}

TEST(SimpleSQL_Tests, CreateTest2) {
  std::string create =
      "create table cats (\n"
      "  id INT NOT NULL PRIMARY KEY,\n"
      "  name VARCHAR(20) NOT NULL,\n"
      "  info VARCHAR(30)\n"
      ");";

  sql::Database db;
  sql::Table created_table = db.ParseRequest(create);
  EXPECT_THROW(db.ParseRequest(create), std::runtime_error);
  // Table cats already exists
}

TEST(SimpleSQL_Tests, CreateTest3) {
  std::string create =
      "Create Table cats (\n"
      "   id INT NOT NULL PRIMARY KEY,"
      "   name VARCHAR(20) NOT NULL,"
      "   is_small BOOL,"
      "   weight FLOAT,"
      "   height DOUBLE"
      ");";

  sql::Database db;
  sql::Table table = db.ParseRequest(create);

  ASSERT_EQ(table.column_label.size(), 5);
  ASSERT_EQ(table.column_label[0], "id");
  ASSERT_EQ(table.column_label[1], "name");
  ASSERT_EQ(table.column_label[2], "is_small");
  ASSERT_EQ(table.column_label[3], "weight");
  ASSERT_EQ(table.column_label[4], "height");

  ASSERT_EQ(table.column_type.size(), 5);
  ASSERT_EQ(table.column_type[0], INT);
  ASSERT_EQ(table.column_type[1], VARCHAR);
  ASSERT_EQ(table.column_type[2], BOOL);
  ASSERT_EQ(table.column_type[3], FLOAT);
  ASSERT_EQ(table.column_type[4], DOUBLE);
}

TEST(SimpleSQL_Tests, InsertTest1) {
  std::string create =
      "CREATE TABLE cats (\n"
      "   id INT NOT NULL PRIMARY KEY,"
      "   name VARCHAR(20) NOT NULL,"
      "   is_small BOOL,"
      "   weight FLOAT,"
      "   height DOUBLE"
      ");";

  sql::Database db;
  db.ParseRequest(create);

  std::string insert1 =
      "INSERT INTO cats "
      "VALUES(1, \"Barsik\", true, 1.6, 0.3);";

  sql::Table table = db.ParseRequest(insert1);

  ASSERT_EQ(table.rows.size(), 1);
  ASSERT_EQ(std::get<int>(table.rows[0][0]), 1);
  ASSERT_EQ(std::get<std::string>(table.rows[0][1]), "Barsik");
  ASSERT_EQ(std::get<bool>(table.rows[0][2]), true);
  ASSERT_EQ(std::get<float>(table.rows[0][3]), 1.6f);
  ASSERT_EQ(std::get<double>(table.rows[0][4]), 0.3);
}

TEST(SimpleSQL_Tests, InsertTest2) {
  std::string create =
      "CREATE TABLE cats (\n"
      "   id INT NOT NULL PRIMARY KEY,"
      "   name VARCHAR(20) NOT NULL,"
      "   is_small BOOL,"
      "   weight FLOAT,"
      "   height DOUBLE"
      ");";

  sql::Database db;
  db.ParseRequest(create);

  std::string insert2 =
      "INSERT INTO cats(id,name) "
      "VALUES(1, \"Barsik\");";

  sql::Table table = db.ParseRequest(insert2);

  ASSERT_EQ(table.rows.size(), 1);
  ASSERT_EQ(std::get<int>(table.rows[0][0]), 1);
  ASSERT_EQ(std::get<std::string>(table.rows[0][1]), "Barsik");
  ASSERT_EQ(std::get<std::nullptr_t>(table.rows[0][2]), nullptr);
  ASSERT_EQ(std::get<std::nullptr_t>(table.rows[0][3]), nullptr);
  ASSERT_EQ(std::get<std::nullptr_t>(table.rows[0][4]), nullptr);
}

TEST(SimpleSQL_Tests, InsertTest3) {
  std::string create =
      "CREATE TABLE cats (\n"
      "   id INT NOT NULL PRIMARY KEY,"
      "   name VARCHAR(20) NOT NULL,"
      "   is_small BOOL,"
      "   weight FLOAT,"
      "   height DOUBLE"
      ");";

  sql::Database db;
  db.ParseRequest(create);

  std::string insert3 =
      "INSERT INTO cats(name, is_small) "
      "VALUES(\"Barsik\", false);";

  EXPECT_THROW(db.ParseRequest(insert3), std::runtime_error);
  // Column id in cats cannot be NULL
}

TEST(SimpleSQL_Tests, InsertTest4) {
  std::string create =
      "CREATE TABLE cats (\n"
      "   id INT NOT NULL PRIMARY KEY,"
      "   name VARCHAR(20) NOT NULL,"
      "   is_small BOOL,"
      "   weight FLOAT,"
      "   height DOUBLE"
      ");";

  sql::Database db;
  db.ParseRequest(create);

  std::string insert4 =
      "iNsErT iNTo cats(id,name,is_small) "
      "valUES(1,\"Barsik\",false);";

  sql::Table table = db.ParseRequest(insert4);

  ASSERT_EQ(table.rows.size(), 1);
  ASSERT_EQ(std::get<int>(table.rows[0][0]), 1);
  ASSERT_EQ(std::get<std::string>(table.rows[0][1]), "Barsik");
  ASSERT_EQ(std::get<bool>(table.rows[0][2]), false);
}

TEST(SimpleSQL_Tests, MultiInsertTest1) {
  std::string create =
      "CREATE TABLE cats (\n"
      "   id INT NOT NULL PRIMARY KEY,"
      "   name VARCHAR(20) NOT NULL,"
      "   is_small BOOL,"
      "   weight FLOAT,"
      "   height DOUBLE"
      ");";

  sql::Database db;
  db.ParseRequest(create);

  std::string insert =
      "INSERT INTO cats(id,name,is_small) VALUES"
      "(1,\"Barsik\",false),"
      "(2,\"Bars\",true),"
      "(3,\"Kot\",true),"
      "(4,\"Catty\",false);";

  sql::Table table = db.ParseRequest(insert);

  ASSERT_EQ(table.rows.size(), 4);

  ASSERT_EQ(std::get<int>(table.rows[0][0]), 1);
  ASSERT_EQ(std::get<std::string>(table.rows[0][1]), "Barsik");
  ASSERT_EQ(std::get<bool>(table.rows[0][2]), false);

  ASSERT_EQ(std::get<int>(table.rows[1][0]), 2);
  ASSERT_EQ(std::get<std::string>(table.rows[1][1]), "Bars");
  ASSERT_EQ(std::get<bool>(table.rows[1][2]), true);

  ASSERT_EQ(std::get<int>(table.rows[2][0]), 3);
  ASSERT_EQ(std::get<std::string>(table.rows[2][1]), "Kot");
  ASSERT_EQ(std::get<bool>(table.rows[2][2]), true);

  ASSERT_EQ(std::get<int>(table.rows[3][0]), 4);
  ASSERT_EQ(std::get<std::string>(table.rows[3][1]), "Catty");
  ASSERT_EQ(std::get<bool>(table.rows[3][2]), false);
}

TEST(SimpleSQL_Tests, MultiInsertTest2) {
  std::string create =
      "CREATE TABLE cats (\n"
      "   id INT NOT NULL PRIMARY KEY,"
      "   name VARCHAR(20) NOT NULL,"
      "   is_small BOOL,"
      "   weight FLOAT,"
      "   height DOUBLE"
      ");";

  sql::Database db;
  db.ParseRequest(create);

  std::string insert =
      "INSERT INTO cats(id,name,is_small) VALUES"
      "(1,\"Barsik\",false),"
      "(1,\"Bars\",true),"
      "(1,\"Kot\",true),"
      "(1,\"Catty\",false);";

  EXPECT_THROW(db.ParseRequest(insert), std::runtime_error);
  // Inserting repeated PRIMARY KEY in table cats
}

TEST(SimpleSQL_Tests, DropTest) {
  std::string create1 = "CREATE TABLE cats1(id INT);";
  std::string create2 = "CREATE TABLE cats2(id INT);";
  std::string create3 = "CREATE TABLE cats3(id INT);";

  sql::Database db;

  db.ParseRequest(create1);
  db.ParseRequest(create2);
  db.ParseRequest(create3);

  std::string drop1 = "DROP TABLE cats1;";
  db.ParseRequest(drop1);
  ASSERT_FALSE(db.tables.contains("cats1"));
  ASSERT_TRUE(db.tables.contains("cats2"));
  ASSERT_TRUE(db.tables.contains("cats3"));

  std::string drop2 = "drop table cats2;";
  db.ParseRequest(drop2);
  ASSERT_FALSE(db.tables.contains("cats1"));
  ASSERT_FALSE(db.tables.contains("cats2"));
  ASSERT_TRUE(db.tables.contains("cats3"));

  std::string drop3 = "Drop Table cats3;";
  db.ParseRequest(drop3);
  ASSERT_FALSE(db.tables.contains("cats1"));
  ASSERT_FALSE(db.tables.contains("cats2"));
  ASSERT_FALSE(db.tables.contains("cats3"));

  std::string drop4 = "DROP TABLE cats1;";
  EXPECT_THROW(db.ParseRequest(drop4), std::runtime_error);
  // Table cats1 does not exist in database
}

TEST(SimpleSQL_Tests, DeleteTest) {
  std::string create =
      "CREATE TABLE cats (\n"
      "   id INT NOT NULL PRIMARY KEY,"
      "   name VARCHAR(20) NOT NULL,"
      "   is_small BOOL,"
      "   weight FLOAT,"
      "   height DOUBLE"
      ");";

  sql::Database db;
  db.ParseRequest(create);

  std::string insert =
      "INSERT INTO cats(id,name,is_small) VALUES"
      "(1,\"Barsik\",false),"
      "(2,\"Bars\",true),"
      "(3,\"Kot\",true),"
      "(4,\"Catty\",false);";

  db.ParseRequest(insert);

  std::string delete_1 = "DELETE FROM cats WHERE cats.id > 2;";
  sql::Table table = db.ParseRequest(delete_1);

  ASSERT_EQ(table.rows.size(), 2);

  ASSERT_EQ(std::get<int>(table.rows[0][0]), 1);
  ASSERT_EQ(std::get<std::string>(table.rows[0][1]), "Barsik");
  ASSERT_EQ(std::get<bool>(table.rows[0][2]), false);

  ASSERT_EQ(std::get<int>(table.rows[1][0]), 2);
  ASSERT_EQ(std::get<std::string>(table.rows[1][1]), "Bars");
  ASSERT_EQ(std::get<bool>(table.rows[1][2]), true);

  std::string delete_2 = "DELETE FROM cats;";
  table = db.ParseRequest(delete_2);
  ASSERT_TRUE(table.rows.empty());
}

TEST(SimpleSQL_Tests, UpdateTest) {
  std::string create =
      "CREATE TABLE cats (\n"
      "   id INT NOT NULL PRIMARY KEY,"
      "   name VARCHAR(20) NOT NULL,"
      "   is_small BOOL,"
      "   weight FLOAT,"
      "   height DOUBLE"
      ");";

  sql::Database db;
  db.ParseRequest(create);

  std::string insert =
      "INSERT INTO cats(id,name,is_small) VALUES"
      "(1,\"Barsik\",false),"
      "(2,\"Bars\",true),"
      "(3,\"Kot\",true),"
      "(4,\"Bot\",false),"
      "(5,\"Cot\",true),"
      "(6,\"Kit\",false),"
      "(7,\"Kitty\",true),"
      "(8,\"Catty\",false);";

  db.ParseRequest(insert);

  std::string update =
      "UPDATE cats "
      "SET cats.name = \"CAT_UPDATED\" "
      "WHERE is_small = true;";
  sql::Table table = db.ParseRequest(update);

  ASSERT_EQ(table.rows.size(), 8);
  ASSERT_EQ(std::get<std::string>(table.rows[1][1]), "CAT_UPDATED");
  ASSERT_EQ(std::get<std::string>(table.rows[2][1]), "CAT_UPDATED");
  ASSERT_EQ(std::get<std::string>(table.rows[4][1]), "CAT_UPDATED");
  ASSERT_EQ(std::get<std::string>(table.rows[6][1]), "CAT_UPDATED");

  std::string update2 =
      "UPDATE cats "
      "SET id = 1000 "
      "WHERE cats.name = \"Barsik\";";
  table = db.ParseRequest(update2);
  ASSERT_EQ(std::get<int>(table.rows[0][0]), 1000);

  std::string update_primary = "update cats set id = 1;";
  EXPECT_THROW(db.ParseRequest(update_primary), std::runtime_error);
  // Update caused repeated PRIMARY KEY in table cats
}

TEST(SimpleSQL_Tests, SaveLoadTest) {
  std::string create1 =
      "CREATE TABLE cats (\n"
      "   id INT NOT NULL PRIMARY KEY,"
      "   name VARCHAR(20) NOT NULL,"
      "   is_small BOOL,"
      "   weight FLOAT,"
      "   height DOUBLE"
      ");";
  sql::Database db;
  db.ParseRequest(create1);

  std::string create2 = "CREATE TABLE shop (id INT);";
  db.ParseRequest(create2);

  std::string insert1 =
      "INSERT INTO cats "
      "VALUES"
      "(1, \"Barsik\", true, 1.6, 0.3),"
      "(2, \"Bars\", false, 2.8, 0.22),"
      "(3, \"Kat\", true, 0.63, 0.13),"
      "(4, \"Cot\", false, 1.28, 0.8);";
  db.ParseRequest(insert1);

  std::string insert2 = "INSERT INTO shop VALUES(1);";
  db.ParseRequest(insert2);

  db.SaveToFile("TestSaving.txt");

  sql::Database loaded_db;
  loaded_db.LoadFrom("TestSaving.txt");

  ASSERT_EQ(db.tables.size(), loaded_db.tables.size());
  for (auto& kv : db.tables) {
    ASSERT_EQ(kv.second.label, loaded_db.tables[kv.first].label);
    ASSERT_EQ(kv.second.column_label, loaded_db.tables[kv.first].column_label);
    ASSERT_EQ(kv.second.column_type, loaded_db.tables[kv.first].column_type);
    ASSERT_EQ(kv.second.column_isnull, loaded_db.tables[kv.first].column_isnull);
    ASSERT_EQ(kv.second.rows, loaded_db.tables[kv.first].rows);
    ASSERT_EQ(kv.second.primary_key_indexes, loaded_db.tables[kv.first].primary_key_indexes);
    ASSERT_EQ(kv.second.m_index_varsize, loaded_db.tables[kv.first].m_index_varsize);
  }
}

TEST(SimpleSQL_Tests, SelectTest1) {
  std::string create1 =
      "CREATE TABLE cats (\n"
      "   id INT NOT NULL PRIMARY KEY,"
      "   name VARCHAR(20) NOT NULL,"
      "   is_small BOOL,"
      "   weight FLOAT,"
      "   height DOUBLE"
      ");";
  sql::Database db;
  db.ParseRequest(create1);

  std::string insert1 =
      "INSERT INTO cats "
      "VALUES"
      "(1, \"Barsik\", true, 1.01, 0.3),"
      "(2, \"Bars\", false, 2.91, 0.22),"
      "(3, \"Kat\", true, 0.72, 0.13),"
      "(4, \"Kat\", true, 0.63, 0.13),"
      "(5, \"Cot\", false, 3.2, 0.8);";
  db.ParseRequest(insert1);

  std::string select1 =
      "SELECT id, name FROM cats WHERE 1 < weight;";
  sql::Table table = db.ParseRequest(select1);

  ASSERT_EQ(table.rows.size(), 3);
  ASSERT_EQ(table.column_label.size(), 2);
  ASSERT_EQ(table.column_type.size(), 2);

  ASSERT_EQ(std::get<int>(table.rows[0][0]), 1);
  ASSERT_EQ(std::get<int>(table.rows[1][0]), 2);
  ASSERT_EQ(std::get<int>(table.rows[2][0]), 5);

  ASSERT_EQ(std::get<std::string>(table.rows[0][1]), "Barsik");
  ASSERT_EQ(std::get<std::string>(table.rows[1][1]), "Bars");
  ASSERT_EQ(std::get<std::string>(table.rows[2][1]), "Cot");
}

TEST(SimpleSQL_Tests, SelectTest2) {
  std::string create1 =
      "CREATE TABLE cats (\n"
      "   id INT NOT NULL PRIMARY KEY,"
      "   name VARCHAR(20) NOT NULL,"
      "   is_small BOOL,"
      "   weight FLOAT,"
      "   height DOUBLE"
      ");";
  sql::Database db;
  db.ParseRequest(create1);

  std::string insert1 =
      "INSERT INTO cats "
      "VALUES"
      "(1, \"Barsik\", true, 1.01, 0.3),"
      "(2, \"Bars\", false, 2.91, 0.22),"
      "(3, \"Kat\", true, 0.72, 0.13),"
      "(4, \"Kat\", true, 0.63, 0.13),"
      "(5, \"Cot\", false, 3.2, 0.8);";
  db.ParseRequest(insert1);

  std::string select2 =
      "SELECT id, name FROM cats WHERE 1 < weight AND is_small = true;";
  sql::Table table = db.ParseRequest(select2);

  ASSERT_EQ(table.rows.size(), 1);
  ASSERT_EQ(table.column_label.size(), 2);
  ASSERT_EQ(table.column_type.size(), 2);

  ASSERT_EQ(std::get<int>(table.rows[0][0]), 1);
  ASSERT_EQ(std::get<std::string>(table.rows[0][1]), "Barsik");
}

TEST(SimpleSQL_Tests, SelectTest3) {
  std::string create1 =
      "CREATE TABLE cats (\n"
      "   id INT NOT NULL PRIMARY KEY,"
      "   name VARCHAR(20) NOT NULL,"
      "   is_small BOOL,"
      "   weight FLOAT,"
      "   height DOUBLE"
      ");";
  sql::Database db;
  db.ParseRequest(create1);

  std::string insert1 =
      "INSERT INTO cats "
      "VALUES"
      "(1, \"Barsik\", true, 1.01, 0.3),"
      "(2, \"Bars\", false, 2.91, 0.22),"
      "(3, \"Kat\", true, 0.72, 0.13),"
      "(4, \"Kat\", true, 5.3, 0.13),"
      "(5, \"Cot\", false, 3.2, 0.8);";
  db.ParseRequest(insert1);

  std::string select2 =
      "SELECT id, name, weight FROM cats "
      "WHERE name in (\"Kat\",\"Barsik\",\"Bars\") AND (id > 3 OR weight < 2.5);";
  sql::Table table = db.ParseRequest(select2);

  ASSERT_EQ(table.rows.size(), 3);
  ASSERT_EQ(table.column_label.size(), 3);
  ASSERT_EQ(table.column_type.size(), 3);

  ASSERT_EQ(std::get<int>(table.rows[0][0]), 1);
  ASSERT_EQ(std::get<int>(table.rows[1][0]), 3);
  ASSERT_EQ(std::get<int>(table.rows[2][0]), 4);

  ASSERT_EQ(std::get<std::string>(table.rows[0][1]), "Barsik");
  ASSERT_EQ(std::get<std::string>(table.rows[1][1]), "Kat");
  ASSERT_EQ(std::get<std::string>(table.rows[2][1]), "Kat");

  ASSERT_EQ(std::get<float>(table.rows[0][2]), 1.01f);
  ASSERT_EQ(std::get<float>(table.rows[1][2]), 0.72f);
  ASSERT_EQ(std::get<float>(table.rows[2][2]), 5.3f);
}

TEST(SimpleSQL_Tests, SelectTest4) {
  std::string create1 =
      "CREATE TABLE cats (\n"
      "   id INT NOT NULL PRIMARY KEY,"
      "   name VARCHAR(20) NOT NULL,"
      "   is_small BOOL,"
      "   weight FLOAT,"
      "   height DOUBLE"
      ");";
  sql::Database db;
  db.ParseRequest(create1);

  std::string insert1 =
      "INSERT INTO cats "
      "VALUES"
      "(1, \"Barsik\", true, 1.01, 0.3),"
      "(2, \"Bars\", false, 2.91, 0.22),"
      "(3, \"Kat\", true, 0.72, 0.13);";
  db.ParseRequest(insert1);

  std::string select2 =
      "SELECT id, name, weight FROM cats WHERE 1 = 1;";
  sql::Table table = db.ParseRequest(select2);

  ASSERT_EQ(table.rows.size(), 3);
  ASSERT_EQ(table.column_label.size(), 3);
  ASSERT_EQ(table.column_type.size(), 3);

  ASSERT_EQ(std::get<int>(table.rows[0][0]), 1);
  ASSERT_EQ(std::get<int>(table.rows[1][0]), 2);
  ASSERT_EQ(std::get<int>(table.rows[2][0]), 3);

  ASSERT_EQ(std::get<std::string>(table.rows[0][1]), "Barsik");
  ASSERT_EQ(std::get<std::string>(table.rows[1][1]), "Bars");
  ASSERT_EQ(std::get<std::string>(table.rows[2][1]), "Kat");

  ASSERT_EQ(std::get<float>(table.rows[0][2]), 1.01f);
  ASSERT_EQ(std::get<float>(table.rows[1][2]), 2.91f);
  ASSERT_EQ(std::get<float>(table.rows[2][2]), 0.72f);
}

TEST(SimpleSQL_Tests, ForeignKeyTest) {
  std::string create = "CREATE TABLE Orders (\n"
                       "    OrderID int NOT NULL PRIMARY KEY,\n"
                       "    OrderNumber int NOT NULL,\n"
                       "    PersonID int FOREIGN KEY REFERENCES Persons(PersonID)\n"
                       ");";
  sql::Database db;
  sql::Table table = db.ParseRequest(create);

  ASSERT_EQ(table.foreign_keys.size(), 1);
  std::pair<std::string, std::string> check = std::make_pair("Persons", "PersonID");
  ASSERT_EQ(table.foreign_keys.at("PersonID"), check);
}

TEST(SimpleSQL_Tests, InnerJoinTest) {
  std::string create_1 =
      "CREATE TABLE persons ("
      "   person_id INT NOT NULL PRIMARY KEY,"
      "   name VARCHAR(20),"
      "   position_id INT"
      ");";
  std::string create_2 =
      "CREATE TABLE positions ("
      "   position_id INT NOT NULL PRIMARY KEY,"
      "   title VARCHAR(20)"
      ");";

  sql::Database db;
  db.ParseRequest(create_1);
  db.ParseRequest(create_2);

  std::string insert_1 =
      "INSERT INTO persons VALUES"
      "(1, \"Vladimir\", 1),"
      "(2, \"Tatiana\", 2),"
      "(3, \"Alexandr\", 6),"
      "(4, \"Boris\", 2);";
  std::string insert_2 =
      "INSERT INTO positions VALUES"
      "(1, \"Designer\"),"
      "(2, \"Writer\"),"
      "(3, \"Scientist\");";
  db.ParseRequest(insert_1);
  db.ParseRequest(insert_2);

  std::string join =
      "SELECT person_id, name, positions.position_id, title "
      "FROM persons "
      "INNER JOIN positions "
      "ON persons.position_id = positions.position_id";
  sql::Table table = db.ParseRequest(join);

  ASSERT_EQ(table.rows.size(), 3);

  ASSERT_EQ(std::get<int>(table.rows[0][0]), 1);
  ASSERT_EQ(std::get<std::string>(table.rows[0][1]), "Vladimir");
  ASSERT_EQ(std::get<int>(table.rows[0][2]), 1);
  ASSERT_EQ(std::get<std::string>(table.rows[0][3]), "Designer");

  ASSERT_EQ(std::get<int>(table.rows[1][0]), 2);
  ASSERT_EQ(std::get<std::string>(table.rows[1][1]), "Tatiana");
  ASSERT_EQ(std::get<int>(table.rows[1][2]), 2);
  ASSERT_EQ(std::get<std::string>(table.rows[1][3]), "Writer");

  ASSERT_EQ(std::get<int>(table.rows[2][0]), 4);
  ASSERT_EQ(std::get<std::string>(table.rows[2][1]), "Boris");
  ASSERT_EQ(std::get<int>(table.rows[2][2]), 2);
  ASSERT_EQ(std::get<std::string>(table.rows[2][3]), "Writer");
}

TEST(SimpleSQL_Tests, LeftJoinTest) {
  std::string create_1 =
      "CREATE TABLE persons ("
      "   person_id INT NOT NULL PRIMARY KEY,"
      "   name VARCHAR(20),"
      "   position_id INT"
      ");";
  std::string create_2 =
      "CREATE TABLE positions ("
      "   position_id INT NOT NULL PRIMARY KEY,"
      "   title VARCHAR(20)"
      ");";

  sql::Database db;
  db.ParseRequest(create_1);
  db.ParseRequest(create_2);

  std::string insert_1 =
      "INSERT INTO persons VALUES"
      "(1, \"Vladimir\", 1),"
      "(2, \"Tatiana\", 2),"
      "(3, \"Alexandr\", 6),"
      "(4, \"Boris\", 2);";
  std::string insert_2 =
      "INSERT INTO positions VALUES"
      "(1, \"Designer\"),"
      "(2, \"Writer\"),"
      "(3, \"Scientist\");";
  db.ParseRequest(insert_1);
  db.ParseRequest(insert_2);

  std::string join =
      "SELECT person_id, name, positions.position_id, title "
      "FROM persons "
      "LEFT JOIN positions "
      "ON persons.position_id = positions.position_id";

  sql::Table table = db.ParseRequest(join);

  ASSERT_EQ(table.rows.size(), 4);

  ASSERT_EQ(std::get<int>(table.rows[0][0]), 1);
  ASSERT_EQ(std::get<std::string>(table.rows[0][1]), "Vladimir");
  ASSERT_EQ(std::get<int>(table.rows[0][2]), 1);
  ASSERT_EQ(std::get<std::string>(table.rows[0][3]), "Designer");

  ASSERT_EQ(std::get<int>(table.rows[1][0]), 2);
  ASSERT_EQ(std::get<std::string>(table.rows[1][1]), "Tatiana");
  ASSERT_EQ(std::get<int>(table.rows[1][2]), 2);
  ASSERT_EQ(std::get<std::string>(table.rows[1][3]), "Writer");

  ASSERT_EQ(std::get<int>(table.rows[2][0]), 3);
  ASSERT_EQ(std::get<std::string>(table.rows[2][1]), "Alexandr");
  ASSERT_EQ(std::get<std::nullptr_t>(table.rows[2][2]), nullptr);
  ASSERT_EQ(std::get<std::nullptr_t>(table.rows[2][3]), nullptr);

  ASSERT_EQ(std::get<int>(table.rows[3][0]), 4);
  ASSERT_EQ(std::get<std::string>(table.rows[3][1]), "Boris");
  ASSERT_EQ(std::get<int>(table.rows[3][2]), 2);
  ASSERT_EQ(std::get<std::string>(table.rows[3][3]), "Writer");
}

TEST(SimpleSQL_Tests, RightJoinTest) {
  std::string create_1 =
      "CREATE TABLE persons ("
      "   person_id INT NOT NULL PRIMARY KEY,"
      "   name VARCHAR(20),"
      "   position_id INT"
      ");";
  std::string create_2 =
      "CREATE TABLE positions ("
      "   position_id INT NOT NULL PRIMARY KEY,"
      "   title VARCHAR(20)"
      ");";

  sql::Database db;
  db.ParseRequest(create_1);
  db.ParseRequest(create_2);

  std::string insert_1 =
      "INSERT INTO persons VALUES"
      "(1, \"Vladimir\", 1),"
      "(2, \"Tatiana\", 2),"
      "(3, \"Alexandr\", 6),"
      "(4, \"Boris\", 2);";
  std::string insert_2 =
      "INSERT INTO positions VALUES"
      "(1, \"Designer\"),"
      "(2, \"Writer\"),"
      "(3, \"Scientist\");";
  db.ParseRequest(insert_1);
  db.ParseRequest(insert_2);

  std::string join =
      "SELECT person_id, name, positions.position_id, title "
      "FROM persons "
      "RIGHT JOIN positions "
      "ON persons.position_id = positions.position_id";

  sql::Table table = db.ParseRequest(join);

  ASSERT_EQ(table.rows.size(), 4);

  ASSERT_EQ(std::get<int>(table.rows[0][0]), 1);
  ASSERT_EQ(std::get<std::string>(table.rows[0][1]), "Vladimir");
  ASSERT_EQ(std::get<int>(table.rows[0][2]), 1);
  ASSERT_EQ(std::get<std::string>(table.rows[0][3]), "Designer");

  ASSERT_EQ(std::get<int>(table.rows[1][0]), 2);
  ASSERT_EQ(std::get<std::string>(table.rows[1][1]), "Tatiana");
  ASSERT_EQ(std::get<int>(table.rows[1][2]), 2);
  ASSERT_EQ(std::get<std::string>(table.rows[1][3]), "Writer");

  ASSERT_EQ(std::get<int>(table.rows[2][0]), 4);
  ASSERT_EQ(std::get<std::string>(table.rows[2][1]), "Boris");
  ASSERT_EQ(std::get<int>(table.rows[2][2]), 2);
  ASSERT_EQ(std::get<std::string>(table.rows[2][3]), "Writer");

  ASSERT_EQ(std::get<std::nullptr_t>(table.rows[3][0]), nullptr);
  ASSERT_EQ(std::get<std::nullptr_t>(table.rows[3][1]), nullptr);
  ASSERT_EQ(std::get<int>(table.rows[3][2]), 3);
  ASSERT_EQ(std::get<std::string>(table.rows[3][3]), "Scientist");
}
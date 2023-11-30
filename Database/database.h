// database.h
#ifndef DATABASE_H
#define DATABASE_H

#include <map>
#include <string>
#include <vector>
#include <set>
struct Message {
    std::string sender;
    std::string content;
};

class BTreeIndex {
public:
    void insert(const std::string& key, const std::string& value);
    std::vector<std::string> search(const std::string& value) const;

private:
    std::map<std::string, std::set<std::string>> index;
};

class BTreePager {
public:
    void insert(const std::string& key, const Message& message);
    void printTree() const;
    const std::map<std::string, Message>& getTree() const;

    BTreeIndex senderIndex;

private:
    std::map<std::string, Message> tree;
};

struct TableColumn {
    std::string name;
    std::string type;
};

struct Table {
    std::string name;
    std::vector<TableColumn> columns;
    std::vector<std::vector<std::string>> rows;
};

class SimpleDatabase {
public:
    void createTable(const std::string& tableName, const std::vector<TableColumn>& columns);
    void insertRow(const std::string& tableName, const std::vector<std::string>& values);
    void getAllRows(const std::string& tableName) const;
    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);

    std::string toLower(const std::string& str) const;

private:
    std::map<std::string, Table> tables;
    BTreePager pager;
};

class REPL {
public:
    void start();

private:
    SimpleDatabase db;

    std::vector<std::string> tokenize(const std::string& input);
    void parseAndExecute(const std::vector<std::string>& tokens);
};

#endif // DATABASE_H

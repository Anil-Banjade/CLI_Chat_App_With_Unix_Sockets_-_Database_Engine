//database.cpp

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>
#include "database.h"


// BTreeIndex implementation
void BTreeIndex::insert(const std::string& key, const std::string& value) {
    index[value].insert(key);
}

std::vector<std::string> BTreeIndex::search(const std::string& value) const {
    auto it = index.find(value);
    if (it != index.end()) {
        return { it->second.begin(), it->second.end() };
    }
    return {};
}

// BTreePager implementation
void BTreePager::insert(const std::string& key, const Message& message) {
    tree[key] = message;
    senderIndex.insert(key, message.sender);
}

void BTreePager::printTree() const {
    for (const auto& pair : tree) {
        std::cout << "Key: " << pair.first << ", Sender: " << pair.second.sender << ", Content: " << pair.second.content << std::endl;
    }
}

const std::map<std::string, Message>& BTreePager::getTree() const {
    return tree;
}

// SimpleDatabase implementation
void SimpleDatabase::createTable(const std::string& tableName, const std::vector<TableColumn>& columns) {
    Table newTable;
    newTable.name = tableName;
    newTable.columns = columns;
    tables[toLower(tableName)] = newTable;

    std::cout<<"table created successfully \n";
}


void SimpleDatabase::insertRow(const std::string& tableName, const std::vector<std::string>& values) {
    // Debug print statement
    std::cout << "Inserting row into table: " << tableName << std::endl;

    auto tableIt = tables.find(tableName);

    if (tableIt == tables.end()) {
        // Table doesn't exist, create it
        createTable(tableName, {{"channelName", "text"}, {"clientId", "text"}, {"message", "text"}});
        tableIt = tables.find(tableName);  // Re-check after creation
    }

    if (tableIt != tables.end()) {
        Table& table = tableIt->second;
        if (table.columns.size() == values.size()) {
            table.rows.push_back(values);

            // Save the database to a file after inserting the row
            saveToFile("database.dat");
            std::cout << "Row inserted successfully into table: " << tableName << std::endl;
        } else {
            std::cout << "Number of values doesn't match the number of columns for table: " << tableName << std::endl;
        }
    } else {
        std::cout << "Table does not exist: " << tableName << std::endl;
    }
}





void SimpleDatabase::getAllRows(const std::string& tableName) const {
    std::string lowerTableName=toLower(tableName);

    if (tables.find(lowerTableName) != tables.end()) {
        const Table& table = tables.at(tableName);

        // Print column names
        for (const auto& column : table.columns) {
            std::cout << column.name << "\t";
        }
        std::cout << std::endl;

        // Print rows
        for (const auto& row : table.rows) {
            for (const auto& value : row) {
                std::cout << value << "\t";
            }
            std::cout << std::endl;
        }
    }
    else {
        std::cout << "Table does not exist: " << tableName << std::endl;
    }
}
std::string SimpleDatabase::toLower(const std::string& str) const {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}


void SimpleDatabase::saveToFile(const std::string& filename) const {
    nlohmann::json jsonTables;
    for (const auto& pair : tables) {
        const Table& table = pair.second;
        nlohmann::json jsonTable;

        jsonTable["name"] = table.name;

        // Convert columns to JSON array
        nlohmann::json jsonColumns;
        for (const auto& column : table.columns) {
            nlohmann::json jsonColumn;
            jsonColumn["name"] = column.name;
            jsonColumn["type"] = column.type;
            jsonColumns.push_back(jsonColumn);
        }
        jsonTable["columns"] = jsonColumns;

        jsonTable["rows"] = table.rows;

        jsonTables[pair.first] = jsonTable;
    }

    

    std::ofstream outputFile(filename);
    if (outputFile.is_open()) {
        outputFile << jsonTables.dump(2); // Pretty-print with indentation
        outputFile.flush();  // Flush the stream
        outputFile.close();  // Close the file
        std::cout << "Database saved to file: " << filename << std::endl;
    } else {
        std::cout << "Failed to open file for writing: " << filename << std::endl;
    }
}


void SimpleDatabase::loadFromFile(const std::string& filename) {
    std::ifstream inputFile(filename);
    if (inputFile.is_open()) {
        nlohmann::json jsonTables;
        inputFile >> jsonTables;

        for (const auto& jsonPair : jsonTables.items()) {
            const std::string& tableName = jsonPair.key();
            const nlohmann::json& jsonTable = jsonPair.value();

            Table table;
            
            // Extract the table name from the "name" field within the table's JSON object
            table.name = jsonTable.value("name", "");

            // Convert columns from JSON array to std::vector<TableColumn>
            const nlohmann::json& jsonColumns = jsonTable.value("columns", nlohmann::json::array());
            if (jsonColumns.is_array()) {
                for (const auto& jsonColumn : jsonColumns) {
                    TableColumn column;
                    column.name = jsonColumn.value("name", "");
                    column.type = jsonColumn.value("type", "");
                    table.columns.push_back(column);
                }
            }
            else {
                std::cout << "Invalid format for columns in table: " << tableName << std::endl;
                continue; // Skip this table and move to the next one
            }

            // Check if "rows" key exists and is an array of arrays of strings
            auto rowsIt = jsonTable.find("rows");
            if (rowsIt != jsonTable.end()) {
                if (rowsIt->is_array()) {
                    table.rows = rowsIt->get<std::vector<std::vector<std::string>>>();
                    tables[table.name] = table;  // Use the extracted table name
                }
                else {
                    std::cout << "Invalid format for rows in table: " << tableName << std::endl;
                    std::cout << "Expected 'rows' to be an array." << std::endl;
                    std::cout << "Actual type: " << rowsIt->type_name() << std::endl;
                }
            }
            else {
                std::cout << "'rows' key not found in table: " << tableName << std::endl;
            }
        }

        std::cout << "Database loaded from file: " << filename << std::endl;
    }
    else {
        std::cout << "Failed to open file: " << filename << std::endl;
    }
}




// REPL (Read-Eval-Print Loop) implementation
void REPL::start() {

    db.loadFromFile("database.dat");
    std::string input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }

        std::vector<std::string> tokens = tokenize(input);
        parseAndExecute(tokens);
    }
}

std::vector<std::string> REPL::tokenize(const std::string& input) {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(input);
    std::string token;
    while (std::getline(tokenStream, token, ' ')) {
        tokens.push_back(token);
    }
    return tokens;
}

void REPL::parseAndExecute(const std::vector<std::string>& tokens) {
    if (tokens.empty()) {
        return;
    }

    const std::string& command = tokens[0];

    if (command == "create_table") {
        if (tokens.size() >= 4) {
            std::string tableName = tokens[1];

            // Extract column names from the string between '(' and ')'
            size_t openParenPos = tokens[2].find('(');
            size_t closeParenPos = tokens.back().find(')');

            if (openParenPos != std::string::npos && closeParenPos != std::string::npos) {
                std::string columnsString = tokens[2].substr(openParenPos + 1, closeParenPos - openParenPos - 1);

                // Tokenize the columns string using ','
                std::vector<std::string> columnNames;
                std::istringstream columnStream(columnsString);
                std::string columnName;
                while (std::getline(columnStream, columnName, ',')) {
                    // Trim leading and trailing whitespaces
                    size_t first = columnName.find_first_not_of(" \t\n");
                    size_t last = columnName.find_last_not_of(" \t\n");
                    if (first != std::string::npos && last != std::string::npos) {
                        columnNames.push_back(columnName.substr(first, last - first + 1));
                    }
                }

                // Convert std::vector<std::string> to std::vector<TableColumn>
                std::vector<TableColumn> tableColumns;
                for (const auto& colName : columnNames) {
                    tableColumns.push_back({colName, "text"});
                }

                db.createTable(tableName, tableColumns);
            } else {
                std::cout << "Invalid create_table command. Usage: create_table <table_name> (<column1, column2, ...>)\n";
            }
        } else {
            std::cout << "Invalid create_table command. Usage: create_table <table_name> (<column1, column2, ...>)\n";
        }
    }

else if (command == "insert") {
    // Debug print the tokens
    std::cout << "Tokens: ";
    for (const auto& token : tokens) {
        std::cout << token << " ";
    }
    std::cout << std::endl;

    // Check if tokens.size() is large enough to prevent access errors
    if (tokens.size() >= 4) {
        std::string tableName = tokens[1];
        std::cout << "inside first if \n";
        std::cout << "table name is " << tableName << "\n";

        // Debug print the sizes and characters in tokens[2] and tokens.back()
        std::cout << "tokens[2].size(): " << tokens[2].size() << ", tokens[2][1]: " << tokens[2][1] << std::endl;
        std::cout << "tokens.back().size(): " << tokens.back().size() << ", tokens.back()[0]: " << tokens.back()[0] << std::endl;

        // Check if the syntax is as expected
         if (tokens[2].size() >= 2 && tokens[2][0] == '(' && tokens.back().size() >= 3 && tokens.back()[tokens.back().size() - 1] == ')') {
            std::cout << "inside another if \n";
            std::vector<std::string> values(tokens.begin() + 2, tokens.end());


          std::cout << "Values vector: ";
        for (const auto& value : values) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
            // Remove ')' from the last value
            values.back().pop_back();

          

            // Call insertRow with the correct vector
            db.insertRow(tableName, values);
            std::cout << "Row inserted successfully into table: " << tableName << std::endl;
            return;
        }
    }
} 




   






    else if (command == "getall") {
        if (tokens.size() == 2) {
            std::string tableName = tokens[1];
            db.getAllRows(tableName);
        }
        else {
            std::cout << "Invalid getall command. Usage: getall <table_name>\n";
        }
    }
    else if (command == "save") {
        if (tokens.size() == 2) {
            db.saveToFile(tokens[1]);
        }
        else {
            std::cout << "Invalid save command. Usage: save <filename>\n";
        }
    }
    else if (command == "load") {
        if (tokens.size() == 2) {
            db.loadFromFile(tokens[1]);
        }
        else {
            std::cout << "Invalid load command. Usage: load <filename>\n";
        }
    }
    else {
        std::cout << "Unknown command: " << command << std::endl;
    }
}




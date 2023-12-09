// database.cpp

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>
#include "database.h"

// BTreeIndex implementation
void BTreeIndex::insert(const std::string &key, const std::string &value)
{
    index[value].insert(key);
}

std::vector<std::string> BTreeIndex::search(const std::string &value) const
{
    auto it = index.find(value);
    if (it != index.end())
    {
        return {it->second.begin(), it->second.end()};
    }
    return {};
}

// BTreePager implementation
void BTreePager::insert(const std::string &key, const Message &message)
{
    tree[key] = message;
    senderIndex.insert(key, message.sender);
}

void BTreePager::printTree() const
{
    for (const auto &pair : tree)
    {
        std::cout << "Key: " << pair.first << ", Sender: " << pair.second.sender << ", Content: " << pair.second.content << std::endl;
    }
}

const std::map<std::string, Message> &BTreePager::getTree() const
{
    return tree;
}

// SimpleDatabase implementation
// void SimpleDatabase::createTable(const std::string& tableName, const std::vector<TableColumn>& columns) {
//     Table newTable;
//     newTable.name = tableName;
//     newTable.columns = columns;
//     tables[toLower(tableName)] = newTable;

//     std::cout<<"table created successfully \n";
// }

void SimpleDatabase::createTable(const std::string &tableName, const std::vector<TableColumn> &columns)
{
    Table newTable;
    newTable.name = tableName;

    // Iterate through the provided columns
    for (const auto &column : columns)
    {
        // Check if the column type ends with ',' and remove it if necessary
        std::string colType = column.type;
        if (!colType.empty() && colType.back() == ',')
        {
            colType.pop_back();
        }

        // Add the column to the new table
        newTable.columns.push_back({column.name, colType});
    }

    tables[toLower(tableName)] = newTable;

    std::cout << "Table created successfully: " << tableName << std::endl;
}



void SimpleDatabase::insertRow(const std::string &tableName, const std::vector<std::string> &values)
{
    // Debug print statement
    std::cout << "Inserting row into table: " << tableName << std::endl;

    auto tableIt = tables.find(tableName); // Access tables directly

    if (tableIt == tables.end())
    {
        // Table doesn't exist, create it dynamically based on insert command
        std::vector<TableColumn> dynamicColumns;
        for (const auto &value : values)
        {
            // Find the position of '=' to separate column name and value
            size_t equalPos = value.find('=');
            if (equalPos != std::string::npos && equalPos < value.size() - 1)
            {
                std::string colName = value.substr(0, equalPos);
                std::string colType = "text"; // Assuming a default type of "text"
                dynamicColumns.push_back({colName, colType});
            }
            else
            {
                std::cout << "Invalid value format: " << value << std::endl;
                return;
            }
        }
        createTable(tableName, dynamicColumns);
        tableIt = tables.find(tableName); // Re-check after creation
    }

    if (tableIt != tables.end())
    {
        Table &table = tableIt->second;

        // Debug print the values being inserted
        std::cout << "Inserting values: ";
        for (const auto &value : values)
        {
            std::cout << value << " ";
        }
        std::cout << std::endl;

        if (table.columns.size() == values.size())
        {
            table.rows.push_back(values);

            // Save the database to a file after inserting the row
            saveToFile("database.dat");
            std::cout << "Row inserted successfully into table: " << tableName << std::endl;
        }
        else
        {
            std::cout << "Number of values doesn't match the number of columns for table: " << tableName << std::endl;
        }
    }
    else
    {
        std::cout << "Table does not exist: " << tableName << std::endl;
    }
}

void SimpleDatabase::getAllRows(const std::string &tableName) const
{
    std::string lowerTableName = toLower(tableName);

    if (tables.find(lowerTableName) != tables.end())
    {
        const Table &table = tables.at(lowerTableName);

        // Print column names
        for (const auto &column : table.columns)
        {
            std::cout << column.name << "\t";
        }
        std::cout << std::endl;

        // Print rows
        for (const auto &row : table.rows)
        {
            for (const auto &value : row)
            {
                std::cout << value << "\t";
            }
            std::cout << std::endl;
        }
    }
    else
    {
        std::cout << "Table does not exist: " << tableName << std::endl;
    }
}
std::string SimpleDatabase::toLower(const std::string &str) const
{
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

void SimpleDatabase::saveToFile(const std::string &filename) const
{
    nlohmann::json jsonTables;
    for (const auto &pair : tables)
    {
        const Table &table = pair.second;
        nlohmann::json jsonTable;

        jsonTable["name"] = table.name;

        // Convert columns to JSON array
        nlohmann::json jsonColumns;
        for (const auto &column : table.columns)
        {
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
    if (outputFile.is_open())
    {
        outputFile << jsonTables.dump(2); // Pretty-print with indentation
        outputFile.flush();               // Flush the stream
        outputFile.close();               // Close the file
        std::cout << "Database saved to file: " << filename << std::endl;
    }
    else
    {
        std::cout << "Failed to open file for writing: " << filename << std::endl;
    }
}

void SimpleDatabase::loadFromFile(const std::string &filename)
{
    std::ifstream inputFile(filename);
    if (inputFile.is_open())
    {
        nlohmann::json jsonTables;
        inputFile >> jsonTables;

        for (const auto &jsonPair : jsonTables.items())
        {
            const std::string &tableName = jsonPair.key();
            const nlohmann::json &jsonTable = jsonPair.value();

            Table table;

            // Extract the table name from the "name" field within the table's JSON object
            table.name = jsonTable.value("name", "");

            // Convert columns from JSON array to std::vector<TableColumn>
            const nlohmann::json &jsonColumns = jsonTable.value("columns", nlohmann::json::array());
            if (jsonColumns.is_array())
            {
                for (const auto &jsonColumn : jsonColumns)
                {
                    TableColumn column;
                    column.name = jsonColumn.value("name", "");
                    column.type = jsonColumn.value("type", "");
                    table.columns.push_back(column);
                }
            }
            else
            {
                std::cout << "Invalid format for columns in table: " << tableName << std::endl;
                continue; // Skip this table and move to the next one
            }

            // Check if "rows" key exists and is an array of arrays of strings
            auto rowsIt = jsonTable.find("rows");
            if (rowsIt != jsonTable.end())
            {
                if (rowsIt->is_array())
                {
                    table.rows = rowsIt->get<std::vector<std::vector<std::string>>>();
                    tables[table.name] = table; // Use the extracted table name
                }
                else
                {
                    std::cout << "Invalid format for rows in table: " << tableName << std::endl;
                    std::cout << "Expected 'rows' to be an array." << std::endl;
                    std::cout << "Actual type: " << rowsIt->type_name() << std::endl;
                }
            }
            else
            {
                std::cout << "'rows' key not found in table: " << tableName << std::endl;
            }
        }

        std::cout << "Database loaded from file: " << filename << std::endl;
    }
    else
    {
        std::cout << "Failed to open file: " << filename << std::endl;
    }
}

std::vector<std::string> SimpleDatabase::searchByChannelName(const std::string &channelName) const
{
    std::string lowerChannelName = toLower(channelName);

    if (tables.find("channels") != tables.end())
    {
        const Table &table = tables.at("channels");

        // Create a vector to store the messages of the specified channel
        std::vector<std::string> channelMessages;

        // Assuming the column order is: channelName, clientId, message
        size_t channelNameIndex = 0; // Index of the column containing channel names

        // Find the index of the channelName column
        for (size_t i = 0; i < table.columns.size(); ++i)
        {
            if (table.columns[i].name == "channelName")
            {
                channelNameIndex = i;
                break;
            }
        }

        // Print rows
        for (const auto &row : table.rows)
        {
            // Check if the current row corresponds to the specified channelName
            if (row.size() > channelNameIndex && toLower(row[channelNameIndex]) == lowerChannelName)
            {
                // Assuming the column order is: channelName, clientId, message
                if (row.size() >= 3)
                {
                    // Add the message to the vector
                    channelMessages.push_back(row[2]);
                }
            }
        }

        searchResults = channelMessages;

        return channelMessages;
    }
    else
    {
        return {}; // Return an empty vector if the "channels" table is not found
    }
}

void SimpleDatabase::saveToFileCSV(const std::string &filename, const std::vector<std::string> &searchResults) const
{
    std::ofstream outputFile(filename);
    if (outputFile.is_open())
    {
        // Check if search results are available
        if (!searchResults.empty())
        {
            // Print search results
            for (const auto &message : searchResults)
            {
                outputFile << message << std::endl;
            }

            outputFile.flush(); // Flush the stream
            outputFile.close(); // Close the file
            std::cout << "Search results saved to CSV file: " << filename << std::endl;
            // Do not clear searchResults here; clear it after saving in the REPL or wherever appropriate
        }
        else
        {
            std::cout << "No search results to save." << std::endl;
        }
    }
    else
    {
        std::cout << "Failed to open file for writing: " << filename << std::endl;
    }
}

// REPL (Read-Eval-Print Loop) implementation
void REPL::start()
{

    db.loadFromFile("../server/database.dat");
    std::string input;
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == "exit")
        {
            break;
        }

        std::vector<std::string> tokens = tokenize(input);
        parseAndExecute(tokens);
    }
}


std::vector<std::string> REPL::tokenize(const std::string &input)
{
    std::vector<std::string> tokens;
    std::istringstream tokenStream(input);
    std::string token;

    while (std::getline(tokenStream, token, ' '))
    {
        // Handle quoted values
        if (!token.empty() && (token.front() == '"' || token.front() == '\''))
        {
            std::string quotedToken = token;
            while (std::getline(tokenStream, token, ' '))
            {
                quotedToken += ' ' + token;
                if (!token.empty() && (token.back() == '"' || token.back() == '\''))
                {
                    break;
                }
            }
            tokens.push_back(quotedToken);
        }
        else
        {
            tokens.push_back(token);
        }
    }

    return tokens;
}

void REPL::parseAndExecute(const std::vector<std::string> &tokens)
{
    if (tokens.empty())
    {
        return;
    }

    const std::string &command = tokens[0];

    if (command == "create_table")
    {
        // Check if tokens.size() is large enough to prevent access errors
        if (tokens.size() >= 4)
        {
            std::string tableName = tokens[1];

            // Check if the syntax is as expected
            if (tokens[2].size() >= 2 && tokens[2][0] == '(' && tokens.back().size() >= 1 && tokens.back()[tokens.back().size() - 1] == ')')
            {
                std::vector<std::string> columns(tokens.begin() + 3, tokens.end());

                // Remove ')' from the last column
                columns.back().pop_back();

                // Create a vector to store TableColumn objects
                std::vector<TableColumn> tableColumns;

                // Iterate through the provided columns
                for (const auto &column : columns)
                {
                    // Find the position of ':' to separate column name and type
                    size_t colonPos = column.find(':');
                    if (colonPos != std::string::npos && colonPos < column.size() - 1)
                    {
                        std::string colName = column.substr(0, colonPos);
                        std::string colType = column.substr(colonPos + 1);
                        tableColumns.push_back({colName, colType});
                    }
                    else
                    {
                        std::cout << "Invalid column format: " << column << std::endl;
                        return;
                    }
                }

                // Call createTable with the correct vector
                db.createTable(tableName, tableColumns);
                std::cout << "Table created successfully: " << tableName << std::endl;
                return;
            }
            else
            {
                std::cout << "Invalid create_table command. Usage: create_table <table_name> (<col1:type1, col2:type2, ...>)\n";
            }
        }
    }

    else if (command == "insert")
    {
        // Check if tokens.size() is large enough to prevent access errors
        if (tokens.size() >= 4)
        {
            std::string tableName = tokens[1];

            // Debug print the sizes and characters in tokens[2] and tokens.back()
            std::cout << "Tokens: ";
            for (const auto &token : tokens)
            {
                std::cout << token << " ";
            }
            std::cout << std::endl;

            // Check if the syntax is as expected
            if (tokens[2].size() >= 2 && tokens[2][0] == '(' && tokens.back().size() >= 3 && tokens.back()[tokens.back().size() - 1] == ')')
            {
                std::vector<std::string> values(tokens.begin() + 3, tokens.end());

                // Remove ')' from the last value
                values.back().pop_back();

                // Retrieve the table columns for the specified table using tables directly
                auto tableIt = db.tables.find(tableName); // Use tables directly

                if (tableIt != db.tables.end())
                {
                    const Table &table = tableIt->second;

                    // Create a map to store column-value pairs
                    std::map<std::string, std::string> colValueMap;

                    // Iterate through the provided values
                    for (const auto &value : values)
                    {
                        // Find the position of '=' to separate column name and value
                        size_t equalPos = value.find('=');
                        if (equalPos != std::string::npos && equalPos < value.size() - 1)
                        {
                            std::string colName = value.substr(0, equalPos);
                            std::string colValue = value.substr(equalPos + 1);
                            colValueMap[colName] = colValue;
                        }
                        else
                        {
                            std::cout << "Invalid value format: " << value << std::endl;
                            return;
                        }
                    }

                    // Ensure that the number of provided columns matches the number of table columns
                    if (colValueMap.size() == table.columns.size())
                    {
                        std::vector<std::string> rowValues;

                        // Iterate through the table columns and fetch corresponding values
                        for (const auto &col : table.columns)
                        {
                            auto it = colValueMap.find(col.name);
                            if (it != colValueMap.end())
                            {
                                rowValues.push_back(it->second);
                            }
                            else
                            {
                                std::cout << "Column not provided: " << col.name << std::endl;
                                return;
                            }
                        }
                        // Call insertRow with the correct vector
                        std::cout << "Inserting into table: " << tableName << std::endl;
                        std::cout << "Row values: ";
                        for (const auto &value : rowValues)
                        {
                            std::cout << value << " ";
                        }
                        std::cout << std::endl;

                        // Call insertRow with the correct vector
                        db.insertRow(tableName, rowValues);
                        std::cout << "Row inserted successfully into table: " << tableName << std::endl;
                        return;
                    }
                    else
                    {
                        std::cout << "Number of provided columns does not match the number of table columns for table: " << tableName << std::endl;
                    }
                }
                else
                {
                    std::cout << "Table does not exist: " << tableName << std::endl;
                }
            }
            else
            {
                std::cout << "Invalid insert command. Usage: insert <table_name> (<col1=value1, col2=value2, ...>)\n";
            }
        }
    }

    else if (command == "getall")
    {
        if (tokens.size() == 2)
        {
            std::string tableName = tokens[1];
            db.getAllRows(tableName);
        }
        else
        {
            std::cout << "Invalid getall command. Usage: getall <table_name>\n";
        }
    }

    else if (command == "save")
    {
        if (tokens.size() == 3 && tokens[1] == "search")
        {
            std::string searchQuery = tokens[2];
            std::string extension = ".csv";
            std::string filename = searchQuery + "_search_results";

            // Check if the filename already has the .csv extension
            if (filename.length() < extension.length() || filename.compare(filename.length() - extension.length(), extension.length(), extension) != 0)
            {
                filename += extension;
            }

            // Perform the search and save the results to the CSV file
            std::vector<std::string> searchResults = db.searchByChannelName(searchQuery);
            db.saveToFileCSV(filename, searchResults);
        }
        else
        {
            std::cout << "Invalid save command. Usage: save search <channel_name>\n";
        }
    }

    else if (command == "load")
    {
        if (tokens.size() == 2)
        {
            db.loadFromFile(tokens[1]);
        }
        else
        {
            std::cout << "Invalid load command. Usage: load <filename>\n";
        }
    }

    else if (command == "search")
    {
        if (tokens.size() == 2)
        {
            std::string channelName = tokens[1];
            std::vector<std::string> messages = db.searchByChannelName(channelName);

            // Print the messages
            for (const auto &message : messages)
            {
                std::cout << message << std::endl;
            }
        }
        else
        {
            std::cout << "Invalid search command. Usage: search <channel_name>\n";
        }
    }

    else
    {
        std::cout << "Unknown command: " << command << std::endl;
    }
}



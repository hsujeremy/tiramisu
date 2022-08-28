#include <cassert>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include "storage.h"

std::string Row::serialize() {
    return std::to_string(data) + "," + std::to_string(event_time);
}

Table::Table() {
}

Table::Table(const std::string table_name) {
    name = table_name;
}

void Table::insert_row(const int data, const std::time_t event_time) {
    Row row;
    row.data = data;
    row.event_time = event_time;
    rows.push_back(row);
}

void Table::flush_to_disk() {
    const std::string dirname = "./data";
    struct stat st;
    if (stat(dirname.c_str(), &st) == -1) {
        mkdir(dirname.c_str(), (mode_t)0777);
    }

    std::string filename;
    if (name == "") {
        filename = dirname + "/output.csv";
    } else {
        filename = dirname + "/" + name + ".csv";
    }

    std::ofstream file(filename);
    for (size_t i = 0; i < rows.size(); ++i) {
        file << rows[i].serialize() << '\n';
    }
    file.close();
}

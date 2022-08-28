#ifndef STORAGE_H
#define STORAGE_H

#include <ctime>
#include <iostream>
#include <vector>

struct Row {
    int data;
    std::time_t event_time;
    
    std::string serialize();
};

struct Table {
    std::string name = "";
    std::vector<Row> rows;

    Table();
    Table(const std::string name);
    void insert_row(const int data, const std::time_t event_time);
    void flush_to_disk();
};

#endif

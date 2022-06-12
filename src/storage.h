#ifndef STORAGE_H
#define STORAGE_H

#include <ctime>
#include <iostream>
#include <vector>

struct Row {
  int data;
  std::time_t event_time;
  std::time_t processing_time;
};
struct Table {
  std::vector<Row> rows;
  void insert_row(int data, std::time_t event_time);
};

#endif

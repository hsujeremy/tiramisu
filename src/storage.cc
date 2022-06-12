#include <cassert>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include "storage.h"

void Table::insert_row(int data, std::time_t event_time) {
  Row row;
  row.data = data;
  row.event_time = event_time;
  row.processing_time = std::time(nullptr);;
  rows.push_back(row);
}

void Table::flush_to_disk() {
  struct stat st;
  if (stat("./data", &st) == -1) {
    mkdir("./data", (mode_t)0777);
  }

  std::ofstream file("./data/output.csv");
  for (size_t i = 0; i < rows.size(); ++i) {
    std::string serialized_row = std::to_string(rows[i].data) + ","
                                 + std::to_string(rows[i].event_time) + ","
                                 + std::to_string(rows[i].processing_time) + ",";
    file << serialized_row;
  }
  file.close();
}

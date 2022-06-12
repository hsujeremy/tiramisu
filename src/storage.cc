#include "storage.h"

void Table::insert_row(int data, std::time_t event_time) {
  Row row;
  row.data = data;
  row.event_time = event_time;
  row.processing_time = std::time(nullptr);;
  rows.push_back(row);
}

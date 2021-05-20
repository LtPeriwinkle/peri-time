#include <sqlite3.h>
#include <memory>
#ifndef TZ_DATABASE_H
#define TZ_DATABASE_H

struct sqlite3_delete {
  void operator() (sqlite3 *db) const {sqlite3_close_v2(db);};
};
using sqlite3_handle = std::unique_ptr<sqlite3, sqlite3_delete>;

#endif

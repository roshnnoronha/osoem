#ifndef EXPORTCSV_H
#define EXPORTCSV_H

#include "../data/database.h"
#include "../data/path.h"

using namespace data;

namespace commands {

void exportcsv (Database& db, Path& pth, std::string filename);




} //namespace commands

#endif  // EXPORTCSV_H

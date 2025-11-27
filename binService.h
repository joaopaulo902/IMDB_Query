//
// Created by joaop on 27/11/2025.
//

#ifndef IMDB_QUERY_BINSERVICE_H
#define IMDB_QUERY_BINSERVICE_H
#include "IQuery.h"
#include <stdio.h>

/**
 *
 * @param entry fixed size type that recieves the data input to the archive
 * @param titlesArray dynamically sized type that the data is passed as
 * @param f fileHeader that contains the number of entries
 * @param fp filePointer
 * puts a title into titles.bin\n
 * if puting a standalone title into file, call put_stand_alone_title(Titles entry, ParseTitle titlesArray, FileHeader* f, FILE* fp);
 * * @code
 *
 * @endcode
 */
void put_title(Titles entry, ParseTitle titlesArray, FileHeader* f, FILE* fp);

void put_stand_alone_title(Titles entry, ParseTitle titlesArray, FileHeader* f, FILE* fp);

#endif //IMDB_QUERY_BINSERVICE_H
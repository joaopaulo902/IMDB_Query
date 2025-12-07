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
 * @param dynamicTitle dynamically sized type that the data is passed as
 * @param f fileHeader that contains the number of entries
 * @param fp filePointer
 * puts a title into titles.bin used when inserting titles in bulk\n
 * (*)if inserting a standalone title into file, call put_stand_alone_title(Titles entry, ParseTitle titlesArray, FileHeader* f, FILE* fp);
 * * @code
 *
 * @endcode
 */
void put_title(Title* entry, ParseTitle dynamicTitle, FileHeader* f, FILE* fp);

/**
 *
 * @param entry Disk format entry
 * @param dynamicTitle Loosely formated entry
 * @param f file Header
 * @param fp file pointer
 */
void put_stand_alone_title(Title entry, ParseTitle dynamicTitle, FileHeader* f, FILE* fp);

/**
 *
 * @param entry
 * @param dynamicRating
 * @param fHeader
 * @param fp
 */
void put_rating(Rating entry, ParseRating dynamicRating, FileHeader fHeader, FILE* fp);

/**
 *
 * @param entry
 * @param searchMode
 * @return corresponding entry to the search
 * @code
 * *SEARCH MODE*
 * 0 - name search
 * 1 - id search
 * @endcode
 */
Title get_title(Title entry, int searchMode);

void update_file_header(const FileHeader* fH, char* title);
void add_title_name(char* rawTitle, int id);
Title get_title_by_id(int id);

#endif //IMDB_QUERY_BINSERVICE_H
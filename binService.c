//
// Created by joaop on 27/11/2025.
//

#include "binService.h"
#include <curl/curl.h>
#include <stdio.h>
#include "entities.h"

void put_title(Titles* entry, ParseTitle dynamicTitle, FileHeader* f, FILE* fp) {
    //copy data into fixed size strings
    strncpy(entry->IMDBid, dynamicTitle.id, sizeof(entry->IMDBid) - 1);
    strncpy(entry->type, dynamicTitle.type, sizeof(entry->type) - 1);
    strncpy(entry->primaryTitle, dynamicTitle.primaryTitle, sizeof(entry->primaryTitle) - 1);
    if (dynamicTitle.plot) {
        strncpy(entry->plot, dynamicTitle.plot, sizeof(entry->plot) - 1);
        entry->plot[sizeof(entry->rating) - 1] = '\0';
    } else {
        entry->plot[0] = '\0';  // safe empty string
    }
    //truncate end of data by substituting it with '\0' in case of overflow
    entry->IMDBid[sizeof(entry->IMDBid) - 1] = '\0';
    entry->type[sizeof(entry->type) - 1] = '\0';
    entry->primaryTitle[sizeof(entry->primaryTitle) - 1] = '\0';
    entry->plot[sizeof(entry->plot) - 1] = '\0';

    entry->startYear = dynamicTitle.startYear;
    entry->runtimeSeconds = dynamicTitle.runtimeSeconds;
    entry->rating.aggregateRating = dynamicTitle.rating.aggregateRating;
    entry->rating.voteCount = dynamicTitle.rating.voteCount;
    //write data into file
    if (fwrite(entry, sizeof(Titles), 1, fp) != 1) {
        perror("write error");
    }
}

/**
 *
 * @param entry
 * @param titlesArray
 * @param fHead
 * @param fp
 *
 */
void put_stand_alone_title(Titles entry, ParseTitle titlesArray, FileHeader* fHead, FILE* fp) {

    fseek(fp, 0, SEEK_SET); //goes to beningin of file
    fread(fHead, sizeof(FileHeader), 1, fp); //reads the header
    off_t offset = (off_t)sizeof(FileHeader) + (off_t)sizeof(Titles) * (off_t)fHead->recordCount;
    _fseeki64(fp, offset, SEEK_SET);
    entry.id = ++fHead->recordCount;
    put_title(&entry, titlesArray, fHead, fp);
    fseek(fp, 0, SEEK_SET); //goes to beningin again
    fwrite(fHead , sizeof(FileHeader), 1, fp); //overwrites Header
    fseek(fp, 0, SEEK_END); //goes to eof
}

void update_file_header(const FileHeader* fH, char fileName[]) {
    FILE* binFp = fopen(fileName, "rb+");
    fseek(binFp, 0, SEEK_SET);
    fwrite(fH, sizeof(FileHeader), 1, binFp);
    fclose(binFp);
}
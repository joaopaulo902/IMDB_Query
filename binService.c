//
// Created by joaop on 27/11/2025.
//

#include "binService.h"
#include <curl/curl.h>
#include <stdio.h>
#include "IQuery.h"
#include "entities.h"

void put_title(Titles entry, ParseTitle titlesArray, FileHeader* f, FILE* fp) {
    //copy data into fixed size strings
    strncpy(entry.IMDBid, titlesArray.id, sizeof(entry.IMDBid) - 1);
    strncpy(entry.type, titlesArray.type, sizeof(entry.type) - 1);
    strncpy(entry.primaryTitle, titlesArray.primaryTitle, sizeof(entry.primaryTitle) - 1);
    if (titlesArray.plot) {
        strncpy(entry.plot, titlesArray.plot, sizeof(entry.plot) - 1);
        entry.plot[sizeof(entry.plot) - 1] = '\0';
    } else {
        entry.plot[0] = '\0';  // safe empty string
    }
    //truncate end of data by substituting it with '\0' in case of overflow
    entry.IMDBid[sizeof(entry.IMDBid) - 1] = '\0';
    entry.type[sizeof(entry.type) - 1] = '\0';
    entry.primaryTitle[sizeof(entry.primaryTitle) - 1] = '\0';

    //write data into file
    fwrite(&entry , sizeof(Titles), 1, fp);
}

void put_stand_alone_title(Titles entry, ParseTitle titlesArray, FileHeader* fHead, FILE* fp) {
    fseek(fp, 0, SEEK_SET); //goes to beningin of file
    fread(fHead, sizeof(FileHeader), 1, fp); //reads the header
    fseek(fp, ((sizeof(Titles) * fHead->recordCount) + sizeof(FileHeader)), SEEK_SET); // goes to last valid entry
    entry.id = ++fHead->recordCount; //assigns incremented value of id
    put_title(entry, titlesArray, fHead, fp); //inserts title into file
    fseek(fp, 0, SEEK_SET); //goes to beningin again
    fwrite(fHead , sizeof(FileHeader), 1, fp); //overwrites Header
    fseek(fp, 0, SEEK_END); //goes to eof
}
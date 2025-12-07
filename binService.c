//
// Created by joaop on 27/11/2025.
//

#include "binService.h"
#include <curl/curl.h>
#include <stdio.h>
#include "entities.h"
#include "titleSearch.h"

void put_title(Title* entry, ParseTitle dynamicTitle, FileHeader* f, FILE* fp) {
    // Copiar strings
    strncpy(entry->IMDBid, dynamicTitle.id, sizeof(entry->IMDBid) - 1);
    entry->IMDBid[sizeof(entry->IMDBid) - 1] = '\0';

    strncpy(entry->type, dynamicTitle.type, sizeof(entry->type) - 1);
    entry->type[sizeof(entry->type) - 1] = '\0';

    strncpy(entry->primaryTitle, dynamicTitle.primaryTitle, sizeof(entry->primaryTitle) - 1);
    entry->primaryTitle[sizeof(entry->primaryTitle) - 1] = '\0';

    if (dynamicTitle.plot) {
        strncpy(entry->plot, dynamicTitle.plot, sizeof(entry->plot) - 1);
        entry->plot[sizeof(entry->plot) - 1] = '\0';
    } else {
        entry->plot[0] = '\0';
    }

    // Copiar campos simples
    entry->startYear = dynamicTitle.startYear;
    entry->runtimeSeconds = dynamicTitle.runtimeSeconds;

    entry->rating.aggregateRating = (int32_t) (100 * dynamicTitle.rating.aggregateRating);
    entry->rating.voteCount = dynamicTitle.rating.voteCount;

    // Escreve struct Titles no binário
    if (fwrite(entry, sizeof(Title), 1, fp) != 1) {
        perror("write error");
    }
}


void add_title_name(char* rawTitle, int id) {
    char normalized[256];
    normalize_title(rawTitle, normalized);

    tokenize_and_index(normalized, id);
}

/**
 *
 * @param entry
 * @param titlesArray
 * @param fHead
 * @param fp
 *
 */
void put_stand_alone_title(Title entry, ParseTitle titlesArray, FileHeader* fHead, FILE* fp) {

    // lê header
    fseek(fp, 0, SEEK_SET);
    fread(fHead, sizeof(FileHeader), 1, fp);

    // calcula offset
    off_t offset = sizeof(FileHeader) + sizeof(Title) * fHead->recordCount;
    _fseeki64(fp, offset, SEEK_SET);

    // DEFINIR ID CORRETO AQUI
    entry.id = ++fHead->recordCount;

    // Indexar o título SOMENTE AGORA — quando o ID está correto
    add_title_name(titlesArray.originalTitle, entry.id);

    // escrever o título
    put_title(&entry, titlesArray, fHead, fp);

    // atualiza header
    fseek(fp, 0, SEEK_SET);
    fwrite(fHead, sizeof(FileHeader), 1, fp);
    fseek(fp, 0, SEEK_END);
}


void update_file_header(const FileHeader* fH, char fileName[]) {
    FILE* binFp = fopen(fileName, "rb+");
    fseek(binFp, 0, SEEK_SET);
    fwrite(fH, sizeof(FileHeader), 1, binFp);
    fclose(binFp);
}

Title get_title_by_id(int id) {
    Title title = {0};
    FILE* fp = fopen("titles.bin", "rb");
    if (!fp) {
        perror("Erro abrindo titles.bin");
        return title;
    }

    // CORREÇÃO AQUI
    off_t offset = sizeof(FileHeader) + sizeof(Title) * (id - 1);
    _fseeki64(fp, offset, SEEK_SET);

    fread(&title, sizeof(Title), 1, fp);

    fclose(fp);
    return title;
}

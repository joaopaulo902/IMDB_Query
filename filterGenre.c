#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "entities.h"
#include "filterGenre.h"

#define HASH_SZ 4194304  // 4 million slots, safe

uint64_t hashFunction(uint64_t h) {
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;
    return (uint64_t) h;
}

void insert_genre_index(ParseTitle title, Title entry) {
    char filename[128];

    for (int k = 0; k < title.genresCount; k++) {
        // find genre in genreList
        int genreIndex = -1;
        for (int j = 0; j < GENRE_COUNT; j++)
            if (strcmp(title.genres[k], GENRE_LIST[j]) == 0)
                genreIndex = j;

        if (genreIndex == -1)
            continue;

        // prepare filename
        snprintf(filename, sizeof(filename), "%s.bin", title.genres[k]);

        // open file
        FILE *fp = fopen(filename, "r+b");

        if (!fp) {
            // create new file
            fp = fopen(filename, "w+b");

            FileHeader FH = {0};
            FH.ID = 0x444E494B;
            FH.recordCount = 0;
            FH.createdAt = time(NULL);
            FH.updatedAt = FH.createdAt;

            // write header
            fwrite(&FH, sizeof(FH), 1, fp);

            // pre-allocate all slots as empty
            GenreTitle zero = {0};
            for (int i = 0; i < HASH_SZ; i++)
                fwrite(&zero, sizeof(zero), 1, fp);
        }

        // read header
        FileHeader FH;
        fseek(fp, 0, SEEK_SET);
        fread(&FH, sizeof(FH), 1, fp);

        // compute base offset of table
        long long base = sizeof(FileHeader);

        // compute hash
        uint64_t h = hashFunction(entry.id);
        uint64_t idx = h % HASH_SZ;

        // do linear probing
        while (1) {
            unsigned long long offset = (unsigned long long) base + idx * (unsigned long long) sizeof(GenreTitle);
            _fseeki64(fp, (long long) offset, SEEK_SET);

            GenreTitle slot;
            fread(&slot, sizeof(slot), 1, fp);

            if (slot.titleId == 0) {
                // empty slot — insert
                _fseeki64(fp, (long long) offset, SEEK_SET);
                GenreTitle genreEntry = {0};
                genreEntry.titleId = entry.id;
                fwrite(&genreEntry, sizeof(genreEntry), 1, fp);
                FH.recordCount++;
                break;
            }

            if (slot.titleId == entry.id)
                break; // already present

            // collision
            idx = (idx + 1) % HASH_SZ;
        }

        FH.updatedAt = time(NULL);

        // rewrite header
        fseek(fp, 0, SEEK_SET);
        fwrite(&FH, sizeof(FH), 1, fp);

        fclose(fp);
    }
}

int genre_contains(const char *genre, int64_t id) {
    char filename[128];
    snprintf(filename, sizeof(filename), "%s.bin", genre);

    FILE *fp = fopen(filename, "rb");
    if (!fp) return 0;

    FileHeader fh;
    fread(&fh, sizeof(fh), 1, fp);

    uint64_t h = hashFunction(id) % HASH_SZ;

    for (uint64_t i = 0; i < HASH_SZ; i++) {
        uint64_t slot = (h + i) % HASH_SZ;

        long long offset = sizeof(FileHeader) + slot * sizeof(GenreTitle);
        _fseeki64(fp, offset, SEEK_SET);

        GenreTitle entry;
        fread(&entry, sizeof(entry), 1, fp);

        if (entry.titleId == id) {
            fclose(fp);
            return 1;
        }

        if (entry.titleId == 0) {
            fclose(fp);
            return 0; // slot vazio → não existe
        }
    }

    fclose(fp);
    return 0;
}

int filter_genre_raw(const char* genre, Title** outTitles) {
    FILE* fp = fopen("titles.bin", "rb");
    if (!fp) return 0;

    FileHeader fh;
    fread(&fh, sizeof(fh), 1, fp);

    // Pular o cabeçalho — ESSENCIAL
    fseek(fp, sizeof(FileHeader), SEEK_SET);

    int capacity = 1024;
    int count = 0;

    Title* results = malloc(capacity * sizeof(Title));
    if (!results) {
        fclose(fp);
        return 0;
    }

    Title t;
    while (fread(&t, sizeof(Title), 1, fp) == 1) {

        // membership test O(1)
        if (genre_contains(genre, t.id)) {

            // expandir vetor caso necessário
            if (count == capacity) {
                capacity *= 2;
                results = realloc(results, capacity * sizeof(Title));
            }

            results[count++] = t;
        }
    }

    fclose(fp);

    *outTitles = results;
    return count;
}


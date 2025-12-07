#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "binService.h"
#include "entities.h"

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

    const char *genreList[] = {
        "Action", "Adult", "Adventure", "Animation", "Biography", "Comedy", "Crime",
        "Documentary", "Drama", "Family", "Fantasy", "Film Noir", "Game Show",
        "History", "Horror", "Musical", "Music", "Mystery", "News", "Reality-TV",
        "Romance", "Sci-Fi", "Short", "Sport", "Talk-Show", "Thriller",
        "War", "Western"
    };

    char filename[128];

    int totalGenreCount = sizeof(genreList)/sizeof(*genreList);

    for (int k = 0; k < title.genresCount; k++) {

        // find genre in genreList
        int genreIndex = -1;
        for (int j = 0; j < totalGenreCount; j++)
            if (strcmp(title.genres[k], genreList[j]) == 0)
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
            for (int i=0;i<HASH_SZ;i++)
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
            unsigned long long offset = (unsigned long long) base + idx * (unsigned long long)sizeof(GenreTitle);
            _fseeki64(fp, (long long)offset, SEEK_SET);

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

void test_filter() { //main
    FILE* fp = fopen("Drama.bin", "r");
    fseek(fp, 0, SEEK_SET);
    FileHeader fh;
    GenreTitle entry;
    fread(&fh, sizeof(fh), 1, fp);
    uint64_t h = hashFunction(1) % HASH_SZ;
    for (uint64_t i = 0; i < HASH_SZ; i++) {
        uint64_t slot = (h + i) % HASH_SZ;
        _fseeki64(fp, (long long) sizeof(fh) + (long long)slot * (long long) sizeof(entry), SEEK_SET);
        fread(&entry, sizeof(entry), 1, fp);
        if (entry.titleId == 1) {
            printf("FOUND at slot %llu\n", slot);
            Title title = get_title_by_id(1);
            printf("%s\n Title name", title.primaryTitle);
            break;
        }
        if (entry.titleId == 0) {
            printf("NOT FOUND - empty slot\n");
            break;
        }
    }

    fclose(fp);
}




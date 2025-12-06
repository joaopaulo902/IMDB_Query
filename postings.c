//
// Created by joaop on 05/12/2025.
//

#include "postings.h"

#include "binService.h"
#include "entities.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

void insert_index_into_posting(ParseTitle title, Titles entry, int i) {
    char *genreList[] = {
        "Action", "Adult", "Adventure", "Animation", "Biography", "Comedy", "Crime", "Documentary", "Drama Family",
        "Fantasy", "Film Noir", "Game Show", "History", "Horror", "Musical", "Music", "Mystery", "News", "Reality-TV",
        "Romance",
        "Sci-Fi", "Short", "Sport", "Talk-Show", "Thriller", "War", "Western"
    };
    GenreTitle genreEntry = {0};
    FileHeader FH = {0};
    char fileName[64];
    int totalGenreCount = sizeof(genreList) / sizeof(*genreList);


    for (int k = 0; k < title.genresCount; k++) {
        for (int j = 0; j < totalGenreCount; j++) {
            if (strcmp(title.genres[k], genreList[j]) != 0) continue;
            snprintf(fileName, sizeof(fileName), "%s.bin", title.genres[k]);
            FILE *fp = fopen(fileName, "rb+");
            if (fp == NULL) {
                fp = fopen(fileName, "w");
                FH.ID = title.genres[k][0] + title.genres[k][1] * 2 * 2 + title.genres[k][2] * 3 * 3;
                FH.version = 0;
                FH.recordCount = 0;
                FH.createdAt = time(NULL);
                FH.updatedAt = time(NULL);
                fwrite(&FH, sizeof(FH), 1, fp);
            }
            fseek(fp, 0, SEEK_SET);
            fread(&FH, sizeof(FH), 1, fp);
            genreEntry.titleId = entry.id;
            genreEntry.id = FH.recordCount + i;
            //assign hash function to hashcode
            int hashcode = 1;
            fseek(fp, (long) sizeof(FileHeader) + (long) sizeof(GenreTitle) * hashcode, SEEK_SET);
            GenreTitle currentGenre;
            fread(&currentGenre, sizeof(currentGenre), 1, fp);
            fseek(fp, -1 * (long) sizeof(GenreTitle), SEEK_CUR);
            fwrite(&genreEntry, 1, sizeof(GenreTitle), fp);
            FH.recordCount++;
            FH.updatedAt = time(NULL);
            fclose(fp);
            update_file_header(&FH, fileName);
        }
    }
}

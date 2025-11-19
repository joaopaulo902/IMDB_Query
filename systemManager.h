//
// Created by andri on 19/11/2025.
//

#ifndef SYSTEMMANAGER_H
#define SYSTEMMANAGER_H
#include "entities.h"
#define PAGE_SIZE 10

void initializeSystem();

void clearScreen();

void printHeader(int currentPage, int totalPages);

void printPage(Title *titles, int totalMovies, int currentPage);

void show_info(Title *movies, int totalMovies);

void updateData();

#endif //SYSTEMMANAGER_H

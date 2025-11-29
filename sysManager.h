//
// Created by andri on 29/11/2025.
//

#ifndef SYSMANAGER_H
#define SYSMANAGER_H

#include "sysManager.h"
#include "entities.h"
#include <stdio.h>
#include <stdlib.h>
#define PAGE_SIZE 10

void initialize_system();

void clearScreen();

void printTitleListHeader(int currentPage, int totalPages);

void printPage(Titles *titles, int totalMovies, int currentPage);
void print_menu_options();

void show_info(Titles *movies, int totalMovies);

void updateData();
#endif //SYSMANAGER_H

//
// Created by andri on 29/11/2025.
//

#ifndef SYSMANAGER_H
#define SYSMANAGER_H

#include "sysManager.h"
#include "entities.h"
#define PAGE_SIZE 10

/**
 * Simulate main system loop
 * **/
void initialize_system();

/**
 * Clear console screen and hide cursor
 * **/
void clear_screen();

/**
 * Print header for titles list
 * @param currentPage current page number
 * @param totalPages total number of pages
 */
void print_title_list_header(int currentPage, int totalPages);

/**
 * Print system information header
 */
void print_info_header();

/**
 * Print paginated list of titles
 * @param page list of titles to be printed
 * @param totalMovies count of total movies in the list
 * @param currentPage current page number to be displayed
 */
void print_titles_list(Titles *page, int totalMovies, int currentPage);

/**
 * Print menu options
 */
void print_menu_options();

/**
 * Show system information page
 * @param totalMovies count of total movies in the list
 */
void show_info_page(int totalMovies);
void show_search_page();

/**
 * Basic bubble sort implementation to order titles by year
 * @param titles list of titles to be ordered
 * @param totalMovies count of total movies in the list
 */
void order_by_year(Titles *titles, int totalMovies);

void print_search_page_results(Titles* results, int count, char* term, int currentPage, double elapsedMs);
void print_search_header(char* term, int currentPage, int totalPages, double elapsedMs);

#endif //SYSMANAGER_H

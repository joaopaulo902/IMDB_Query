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
 * Show system information page
 * @param totalMovies count of total movies in the list
 */
void show_info_page(int totalMovies);

/**
 * Show search page and handle user input
 */
void show_search_page();

/**
 * Print paginated list of titles
 * @param page list of titles to be printed
 * @param totalMovies count of total movies in the list
 * @param currentPage current page number to be displayed
 */
void print_titles_list(Title *page, int totalMovies, int currentPage);

/**
 * Print search results page
 * @param results list of titles resulting from the search
 * @param count count of total search results
 * @param term search term used
 * @param currentPage current page number to be displayed
 * @param elapsedMs time taken to perform the search in milliseconds
 */
void print_search_page_results(Title* results, int count, char* term, int currentPage, double elapsedMs);

/**
 * Show genre filter page and handle user input
 */
void show_genre_filter_page();

#endif //SYSMANAGER_H

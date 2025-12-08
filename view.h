#ifndef VIEW_H
#define VIEW_H

#endif //VIEW_H
/**
 * Clear console screen and hide cursor
 * **/
void clear_screen();

/**
 * Read and print title from title.txt
 */
void read_title();

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
 * Print menu options
 */
void print_menu_options();

/**
 * Print header for title search results
 */
void print_title_search_header();

/**
 * Print search results menu options
 */
void print_results_menu();

/**
 * Print genre filter header
 */
void print_genre_filter_header();

/**
 * Print genre results header
 * @param genre genre being filtered
 * @param currentPage current page number to be displayed
 * @param totalPages total number of pages
 */
void print_genre_results_header(const char *genre, int currentPage, int totalPages, double elapsedMs);

/**
 * Print search results header
 * @param term search term used
 * @param currentPage current page number to be displayed
 * @param totalPages total number of pages
 * @param elapsedMs time taken to perform the search in milliseconds
 */
void print_search_header(char *term, int currentPage, int totalPages, double elapsedMs);

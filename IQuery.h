//
// Created by joaop on 21/11/2025.
//

#ifndef IMDB_QUERY_IQUERY_H
#define IMDB_QUERY_IQUERY_H

/** gets info from api referenced by the @code url@endcode and inserts it into the desired @code fileName@endcode
 *
 *- appends every request to the end of the file
*/
int get_info(char* url, const char* fileName);



#endif //IMDB_QUERY_IQUERY_H
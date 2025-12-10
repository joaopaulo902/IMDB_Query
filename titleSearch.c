#include "titleSearch.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

PostingList dictionary[DICT_SIZE] = {0};

char remove_accent(unsigned char c) {
    // UTF-8 lower accents
    if (c == 0xA1 || c == 0xA0 || c == 0xA3 || c == 0xA2 || c == 0xA4) return 'a';
    if (c == 0xA9 || c == 0xA8 || c == 0xAA || c == 0xAB) return 'e';
    if (c == 0xAD || c == 0xAC || c == 0xAE || c == 0xAF) return 'i';
    if (c == 0xB3 || c == 0xB2 || c == 0xB5 || c == 0xB4 || c == 0xB6) return 'o';
    if (c == 0xBA || c == 0xB9 || c == 0xBB || c == 0xBC) return 'u';
    if (c == 0xA7) return 'c';

    // UTF-8 upper accents
    if (c == 0x81 || c == 0x80 || c == 0x83 || c == 0x82 || c == 0x84) return 'a';
    if (c == 0x89 || c == 0x88 || c == 0x8A || c == 0x8B) return 'e';
    if (c == 0x8D || c == 0x8C || c == 0x8E || c == 0x8F) return 'i';
    if (c == 0x93 || c == 0x92 || c == 0x95 || c == 0x94 || c == 0x96) return 'o';
    if (c == 0x9A || c == 0x99 || c == 0x9B || c == 0x9C) return 'u';
    if (c == 0x87) return 'c';

    return tolower(c);
}

void normalize_title(char *input, char *output) {
    int j = 0;
    int last_was_space = 1;

    for (int i = 0; input[i] != '\0'; i++) {
        unsigned char c = input[i];
        char clean = remove_accent(c);

        // Letras, números ou espaços
        if (isalnum(clean)) {
            output[j++] = clean;
            last_was_space = 0;
        } else if (isspace(clean)) {
            // evita múltiplos espaços
            if (!last_was_space) {
                output[j++] = ' ';
                last_was_space = 1;
            }
        } else {
            // ignora símbolos como !, ?, :, -, /, etc.
            continue;
        }
    }

    // remove espaço final
    if (j > 0 && output[j - 1] == ' ')
        j--;

    output[j] = '\0';
}

void tokenize_and_index(char *normalized, int id) {
    // Copy name to a modifiable buffer
    char buffer[256];
    strncpy(buffer, normalized, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';

    // Tokenize by spaces, ignoring stopwords
    // Remove token by token until the end of the word
    char *token = strtok(buffer, " ");
    while (token != NULL) {
        if (!is_stopword(token)) {
            insert_token_into_dictionary(token, id);
        }

        token = strtok(NULL, " ");
    }
}

unsigned int hash_token(char *s) {
    unsigned int h = 2166136261u;
    while (*s) {
        h ^= (unsigned char) (*s++);
        h *= 16777619u;
    }
    return h % DICT_SIZE;
}

void insert_token_into_dictionary(char *token, int id) {
    unsigned int index = hash_token(token);

    while (1) {
        // If empty slot, insert new term
        if (dictionary[index].term == NULL) {
            dictionary[index].term = strdup(token);
            dictionary[index].capacity = 4;
            dictionary[index].count = 0;
            dictionary[index].ids = malloc(4 * sizeof(int));

            dictionary[index].ids[dictionary[index].count++] = id;
            return;
        }

        // If term already exists, append ID
        if (strcmp(dictionary[index].term, token) == 0) {
            // Avoid duplicate IDs
            if (dictionary[index].count == 0 || dictionary[index].ids[dictionary[index].count - 1] != id) {
                // Expand (double) structure if needed
                if (dictionary[index].count == dictionary[index].capacity) {
                    dictionary[index].capacity *= 2;
                    dictionary[index].ids = realloc(dictionary[index].ids,
                                                    dictionary[index].capacity * sizeof(int));
                }

                dictionary[index].ids[dictionary[index].count++] = id;
            }

            return;
        }

        // Linear probing
        index = (index + 1) % DICT_SIZE;
    }
}

int is_stopword(char *token) {
    const char *STOPWORDS[] = {
        "a", "an",
        "of", "and", "or",
        "to", "in", "on",
        "for", "with",
        "de", "da", "do", "das", "dos",
        "o", "os", "as",
        "um", "uma", "uns", "umas",
        "la", "el", "los", "las",
        "y", "e",
        "por",
        NULL
    };

    for (int i = 0; STOPWORDS[i] != NULL; i++) {
        if (strcmp(token, STOPWORDS[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

int64_t write_posting_list(FILE *posts, int *ids, int count) {

    int64_t firstOffset = ftell(posts);

    int remaining = count;
    int indexBase = 0;

    while (remaining > 0) {
        PostingBlock block;

        block.count = remaining > BLOCK_SIZE ? BLOCK_SIZE : remaining;
        block.nextOffset = 0;


        for (int i = 0; i < block.count; i++) {
            block.ids[i] = ids[indexBase + i];
        }

        int64_t blockStart = ftell(posts);

        fwrite(&block.count, sizeof(int), 1, posts);
        fwrite(&block.nextOffset, sizeof(int64_t), 1, posts);
        fwrite(block.ids, sizeof(int), block.count, posts);

        remaining -= block.count;
        indexBase += block.count;

        if (remaining > 0) {
            int64_t next = ftell(posts);
            fseek(posts, blockStart + sizeof(int), SEEK_SET);
            fwrite(&next, sizeof(int64_t), 1, posts);

            fseek(posts, next, SEEK_SET);
        }
    }

    return firstOffset;
}


void save_dictionary(const char *vocabFile, const char *postingsFile) {
    FILE *vocab = fopen(vocabFile, "wb");
    FILE *posts = fopen(postingsFile, "wb");

    if (!vocab || !posts) {
        perror("Erro ao abrir arquivos de índice");
        return;
    }

    // Criar um vetor temporário para armazenar as entradas válidas
    int listCount = 0;
    DictEntry *list = malloc(DICT_SIZE * sizeof(DictEntry));

    for (int i = 0; i < DICT_SIZE; i++) {
        if (dictionary[i].term != NULL) {
            list[listCount].term = dictionary[i].term;
            list[listCount].count = dictionary[i].count;
            list[listCount].ids = dictionary[i].ids;
            listCount++;
        }
    }

    // Ordenar alfabeticamente (no momento não é estável)
    qsort(list, listCount, sizeof(DictEntry), compare_terms);
    //tentativa de ordenação estável
    /*msd_radix_sort_dict(list, listCount);*/

    // Escrever vocabulário e postings
    for (int i = 0; i < listCount; i++) {
        // Opcional: ordenar IDs para deixar mais elegante
        qsort(list[i].ids, list[i].count, sizeof(int), compare_int);

        VocabularyEntry entry = {0};
        strncpy(entry.term, list[i].term, sizeof(entry.term) - 1);

        // Criar lista encadeada no postings.bin
        /*for (int k = 0; k < list[i].count; k++) printf("%d ", list[i].ids[k]);*/

        entry.firstBlockOffset = write_posting_list(posts, list[i].ids, list[i].count);

        fwrite(&entry, sizeof(VocabularyEntry), 1, vocab);
    }

    free(list);
    fclose(vocab);
    fclose(posts);
}


int compare_terms(const void *a, const void *b) {
    const DictEntry *A = (const DictEntry *) a;
    const DictEntry *B = (const DictEntry *) b;
    return strcmp(A->term, B->term);
}

int compare_int(const void *a, const void *b) {
    int A = *(int *) a;
    int B = *(int *) b;
    return A - B;
}


VocabularyEntry find_in_vocabulary(char *term) {
    FILE *fp = fopen("vocabulary.bin", "rb");
    VocabularyEntry entry;

    // valor padrão para "não encontrado"
    entry.firstBlockOffset = -1;

    if (!fp) {
        perror("Erro ao abrir vocabulary.bin");
        return entry;
    }

    // Descobrir número total de entradas
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    int totalEntries = fileSize / sizeof(VocabularyEntry);
    fseek(fp, 0, SEEK_SET);

    int left = 0;
    int right = totalEntries - 1;

    while (left <= right) {
        int mid = (left + right) / 2;

        fseek(fp, mid * sizeof(VocabularyEntry), SEEK_SET);
        fread(&entry, sizeof(VocabularyEntry), 1, fp);

        int cmp = strcmp(term, entry.term);

        if (cmp == 0) {
            fclose(fp);
            return entry;
        } else if (cmp < 0) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    fclose(fp);

    // Não encontrado
    entry.firstBlockOffset = -1;
    return entry;
}

int *load_postings(int64_t offset, int *outCount) {
    FILE *fp = fopen("postings.bin", "rb");
    if (!fp) {
        perror("Erro ao abrir postings.bin");
        *outCount = 0;
        return NULL;
    }

    int total = 0;
    int64_t current = offset;
    PostingBlock block;

    while (current != 0) {
        fseek(fp, current, SEEK_SET);

        fread(&block.count, sizeof(int), 1, fp);
        fread(&block.nextOffset, sizeof(int64_t), 1, fp);

        total += block.count;
        current = block.nextOffset;
    }


    // Alocar vetor para todos IDs
    int *ids = malloc(total * sizeof(int));

    // 2ª PASSAGEM: carregar IDs
    current = offset;
    int pos = 0;


    while (current != 0) {
        fseek(fp, current, SEEK_SET);

        fread(&block.count, sizeof(int), 1, fp);
        fread(&block.nextOffset, sizeof(int64_t), 1, fp);
        fread(block.ids, sizeof(int), block.count, fp);

        memcpy(ids + pos, block.ids, block.count * sizeof(int));
        pos += block.count;

        current = block.nextOffset;
    }

    fclose(fp);

    *outCount = total;
    return ids;
}


int *search_term(char *term, int *outCount) {
    *outCount = 0;

    // 1. Normalizar o termo
    char normalized[256];
    normalize_title((char *) term, normalized);

    // 2. Procurar no vocabulary.bin
    VocabularyEntry entry = find_in_vocabulary(normalized);

    if (entry.firstBlockOffset == -1) {
        // Não encontrado
        return NULL;
    }

    // 3. Carregar postings a partir do primeiro bloco
    return load_postings(entry.firstBlockOffset, outCount);
}

int char_at (const char *s, int d) {
    unsigned char c = s[d];
    return (c == '\0') ? -1 : c;
}

void bubble_sort(DictEntry *a, size_t lo, size_t hi, int d) {
    if (hi <= lo) return;

    for (size_t i = hi; i > lo; i--) {
        for (size_t j = lo; j < i; j++) {

            const char *s1 = get_key(&a[j]);
            const char *s2 = get_key(&a[j + 1]);

            // compare starting at digit d
            int k = d;
            while (char_at(s1, k) == char_at(s2, k) &&
                   char_at(s1, k) >= 0)
                k++;

            if (char_at(s1, k) > char_at(s2, k)) {
                DictEntry tmp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = tmp;
            }
        }
    }
}

void msd_sort_rec(DictEntry *a, DictEntry *aux, size_t lo, size_t hi, int d) {

    if (hi <= lo + CUTOFF) {
        bubble_sort(a, lo, hi, d);
        return;
    }

    size_t count[ASCII_SIZE + 2] = {0};

    // Frequency count
    for (size_t i = lo; i <= hi; i++) {
        int c = char_at(get_key(&a[i]), d) + 2;
        count[c]++;
    }

    // Create cumulative array
    size_t start[ASCII_SIZE + 2];
    start[0] = 0;
    for (int r = 0; r < ASCII_SIZE + 1; r++)
        start[r + 1] = start[r] + count[r];

    // next[] pointers for distribution
    size_t next[ASCII_SIZE + 2];
    memcpy(next, start, sizeof(start));

    // Distribute stably
    for (size_t i = lo; i <= hi; i++) {
        int c = char_at(get_key(&a[i]), d) + 1;
        aux[next[c]++] = a[i];
    }

    // Copy back
    for (size_t i = lo; i <= hi; i++)
        a[i] = aux[i - lo];

    // Recurse into buckets
    for (int r = 0; r < ASCII_SIZE; r++) {
        size_t b_lo = lo + start[r];
        size_t b_hi = lo + start[r + 1] - 1;

        if (b_hi >= b_lo)
            msd_sort_rec(a, aux, b_lo, b_hi, d + 1);
    }
}


void msd_radix_sort_dict(DictEntry *arr, size_t n) {
    if (n <= 1) return;
    DictEntry *aux = malloc(n * sizeof(DictEntry));
    msd_sort_rec(arr, aux, 0, n - 1, 0);
    free(aux);
}

char *get_key(const DictEntry *e) {
    return e->term;        // change this if the key is in another field
}
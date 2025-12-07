cmake_minimum_required(VERSION 3.20)
project(IMDB_Query C)

# ---------------------------------------------
# Dependências (libcurl + cJSON via vcpkg)
# ---------------------------------------------
find_package(CURL REQUIRED)
find_package(cjson CONFIG REQUIRED)

# ---------------------------------------------
# Variáveis de diretório
# ---------------------------------------------
set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resource")

# Garante que o diretório resource exista
file(MAKE_DIRECTORY "${RESOURCE_DIR}")

# ---------------------------------------------
# Executável principal
# ---------------------------------------------
add_executable(IMDB_Query
        main.c
        util.c util.h
        IQuery.c IQuery.h
        entities.c entities.h
        binService.c binService.h
        dbContext.c dbContext.h
        apiHandler.c apiHandler.h
        sysManager.c sysManager.h
        titleSearch.c
        titleSearch.h
        filterGenre.h filterGenre.c
)

# ---------------------------------------------
# Bibliotecas linkadas
# ---------------------------------------------
target_link_libraries(IMDB_Query PRIVATE
        CURL::libcurl
        cjson
)

# ---------------------------------------------
# Definir caminhos para leitura dos arquivos
# ---------------------------------------------
target_compile_definitions(IMDB_Query PRIVATE
        TITLE_FILE_PATH="${RESOURCE_DIR}/title.txt"
        TITLES_BIN_PATH="${RESOURCE_DIR}/titles.bin"
)

# ---------------------------------------------
# Copiar titles.bin do build → para resource/
# Sempre que mudar (melhor para seu fluxo)
# ---------------------------------------------
add_custom_command(
        TARGET IMDB_Query POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Atualizando titles.bin em resource/..."

        # Create file if it doesn't exist
        COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_CURRENT_BINARY_DIR}/${GENRE}.bin"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_BINARY_DIR}/titles.bin"
        "${RESOURCE_DIR}/titles.bin"
        COMMENT "Copiando titles.bin do build para resource/"
)
# Lista de gêneros
set(GENRES
        "Action" "Adult" "Adventure" "Animation" "Biography" "Comedy" "Crime"
        "Documentary" "Drama" "Family" "Fantasy" "Film Noir" "Game Show"
        "History" "Horror" "Musical" "Music" "Mystery" "News" "Reality-TV"
        "Romance" "Sci-Fi" "Short" "Sport" "Talk-Show" "Thriller"
        "War" "Western"
)
# Loop para copiar cada GENRE.bin
foreach(GENRE ${GENRES})
    add_custom_command(
            TARGET IMDB_Query POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Atualizando ${GENRE}.bin em resource/..."

            # Create file if it doesn't exist
            COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_CURRENT_BINARY_DIR}/${GENRE}.bin"

            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/${GENRE}.bin"
            "${RESOURCE_DIR}/${GENRE}.bin"
    )
endforeach()


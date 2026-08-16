cmake_minimum_required(VERSION 3.10)

project(screensaver C)

include(FetchContent)

# ============================================================
# Raylib
# ============================================================

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)

FetchContent_MakeAvailable(raylib)


# ============================================================
# FFmpeg
# ============================================================

set(FFMPEG_VERSION "8.1.2")
set(FFMPEG_URL
    "https://github.com/GyanD/codexffmpeg/releases/download/8.1.2/ffmpeg-8.1.2-full_build.zip"
)

set(FFMPEG_DIR "${CMAKE_BINARY_DIR}/ffmpeg")
set(FFMPEG_ARCHIVE "${CMAKE_BINARY_DIR}/ffmpeg.zip")

# Télécharger FFmpeg si nécessaire
if(NOT EXISTS "${FFMPEG_DIR}/bin/ffmpeg.exe")

    message(STATUS "FFmpeg ${FFMPEG_VERSION} non trouvé.")
    message(STATUS "Téléchargement de FFmpeg...")

    file(DOWNLOAD
        "${FFMPEG_URL}"
        "${FFMPEG_ARCHIVE}"
        SHOW_PROGRESS
        STATUS FFMPEG_DOWNLOAD_STATUS
    )

    list(GET FFMPEG_DOWNLOAD_STATUS 0 FFMPEG_DOWNLOAD_CODE)

    if(NOT FFMPEG_DOWNLOAD_CODE EQUAL 0)
        message(FATAL_ERROR
            "Impossible de télécharger FFmpeg."
        )
    endif()

    message(STATUS "Extraction de FFmpeg...")

    file(MAKE_DIRECTORY "${FFMPEG_DIR}")

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar
                xvf "${FFMPEG_ARCHIVE}"
        WORKING_DIRECTORY "${FFMPEG_DIR}"
        RESULT_VARIABLE FFMPEG_EXTRACT_RESULT
    )

    if(NOT FFMPEG_EXTRACT_RESULT EQUAL 0)
        message(FATAL_ERROR
            "Impossible d'extraire FFmpeg."
        )
    endif()

    # Le ZIP contient normalement :
    # ffmpeg-8.1.2-full_build/bin/ffmpeg.exe
    #
    # On déplace le contenu du dossier vers ffmpeg/
    file(GLOB FFMPEG_EXTRACTED_DIR
        "${FFMPEG_DIR}/ffmpeg-*"
    )

    if(FFMPEG_EXTRACTED_DIR)
        list(GET FFMPEG_EXTRACTED_DIR 0 FFMPEG_ROOT)

        file(COPY
            "${FFMPEG_ROOT}/"
            DESTINATION "${FFMPEG_DIR}"
        )

        file(REMOVE_RECURSE "${FFMPEG_ROOT}")
    endif()

    file(REMOVE "${FFMPEG_ARCHIVE}")

endif()


# ============================================================
# Vérification FFmpeg
# ============================================================

set(FFMPEG_EXECUTABLE
    "${FFMPEG_DIR}/bin/ffmpeg.exe"
)

set(FFPROBE_EXECUTABLE
    "${FFMPEG_DIR}/bin/ffprobe.exe"
)

if(NOT EXISTS "${FFMPEG_EXECUTABLE}")
    message(FATAL_ERROR
        "ffmpeg.exe introuvable : ${FFMPEG_EXECUTABLE}"
    )
endif()

if(NOT EXISTS "${FFPROBE_EXECUTABLE}")
    message(FATAL_ERROR
        "ffprobe.exe introuvable : ${FFPROBE_EXECUTABLE}"
    )
endif()

message(STATUS "FFmpeg : ${FFMPEG_EXECUTABLE}")
message(STATUS "FFprobe : ${FFPROBE_EXECUTABLE}")


# ============================================================
# Programme
# ============================================================

add_executable(main
    src/main.c
    src/inactivity.c
    src/video.c
)

target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)


# ============================================================
# Raylib
# ============================================================

target_link_libraries(main raylib)


# ============================================================
# Chemins FFmpeg accessibles depuis le code C
# ============================================================

target_compile_definitions(main PRIVATE
    FFMPEG_PATH="${FFMPEG_EXECUTABLE}"
    FFPROBE_PATH="${FFPROBE_EXECUTABLE}"
)
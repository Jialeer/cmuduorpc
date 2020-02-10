set(LIBRARY_OUTPUT_PATH &{PROJECT_SOURCE_DIR}/lib/tinyxml)
aux_source_directory(. SRC_LIST)
add_library(tinyxml SHARED ${SRC_LIST})
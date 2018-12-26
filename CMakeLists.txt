cmake_minimum_required(VERSION 3.12)

project(MtkUtil
        DESCRIPTION "Unpack/Repack MTK logo.bin-file"
        HOMEPAGE_URL "https://github.com/ergotamin/mediathek-logo.bin-builder.git"
        LANGUAGES C)

add_definitions(-DGNU_SOURCE -D__IDENT__=${PROJECT_NAME})

include(GNUInstallDirs)

set(MTKUTIL mtkutil)


find_package(ImageMagick REQUIRED COMPONENTS MagickCore MagickWand)
find_package(GTK REQUIRED gtk+-3.0)
find_package(ZLIB REQUIRED)

if((NOT GTK_FOUND) OR (NOT ZLIB_FOUND) OR (NOT ImageMagick_FOUND))
      message(FATAL_ERROR "You are missing required dependencies !")
endif()

file(GLOB_RECURSE MtkUtilHeaders "${CMAKE_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE MtkUtilSources "${CMAKE_SOURCE_DIR}/src/*.c")

add_compile_options(-Wall -Wno-format 
                    -Wno-deprecated -O2 -Ofast 
                    -mtune=native -std=gnu99
                    -ffast-math -fopenmp
                    -DMAGICKCORE_HDRI_ENABLE=1 
                    -DMAGICKCORE_QUANTUM_DEPTH=16)

add_executable(${MTKUTIL} ${MtkUtilSources} ${MtkUtilHeaders} ${CMAKE_INCLUDE_PATH})

target_include_directories(${MTKUTIL} PUBLIC 
      ${CMAKE_SOURCE_DIR}/include ${ImageMagick_INCLUDE_DIRS} 
      ${GTK_INCLUDE_DIR} ${ZLI_INCLUDE_DIRS})

target_link_libraries(${MTKUTIL} PUBLIC 
      ${ImageMagick_LIBRARIES} ${GTK_LIBRARIES} 
      ${ZLIB_LIBRARIES})

install(TARGETS ${MTKUTIL} RUNTIME DESTINATION ${CMAKE_BINARY_DIR})
cmake_minimum_required(VERSION 3.10)
project(Matrix)
set(CMAKE_CXX_STANDARD 11)
set(result_exe "test5_multithread")
set(file1 "test5_multithread.cc")

# Include pkg-config support
include(FindPkgConfig)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread -O3")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(${result_exe} ${file1} modules.cc modules.h draw_tools.h draw_tools.cc database.h database.cc)

find_package(nlohmann_json REQUIRED)
#check if nlohmann_json was found
if(nlohmann_json_FOUND)
    message("nlohmann_json found")
    include_directories(${nlohmann_json_INCLUDE_DIRS})
    target_link_libraries(${result_exe} ${nlohmann_json_LIBRARIES})
else()
    message("nlohmann_json not found")
endif()


find_package(ImageMagick REQUIRED COMPONENTS MagickCore Magick++)
#check if Magick++ was found
if(ImageMagick_FOUND)
    message("ImageMagick found")
    add_definitions( -DMAGICKCORE_QUANTUM_DEPTH=16 )
	add_definitions( -DMAGICKCORE_HDRI_ENABLE=0 )
    include_directories(${ImageMagick_INCLUDE_DIRS})
    target_link_libraries(${result_exe} ${ImageMagick_LIBRARIES})
else()
    message("ImageMagick not found")
endif()


find_package(CURL REQUIRED)
#check if CURL was found
if(CURL_FOUND)
    message("CURL found")
    include_directories(${CURL_INCLUDE_DIRS})
    target_link_libraries(${result_exe} ${CURL_LIBRARIES})
else()
    message("CURL not found")
endif()

pkg_check_modules(LIBGPIOD REQUIRED libgpiod)
include_directories(${LIBGPIOD_INCLUDE_DIRS})
target_link_libraries(${result_exe} ${LIBGPIOD_LIBRARIES})

pkg_check_modules(SQLITE3 REQUIRED sqlite3)
include_directories(${SQLITE3_INCLUDE_DIRS})
target_link_libraries(${result_exe} ${SQLITE3_LIBRARIES})



target_link_libraries(${result_exe} ${CMAKE_SOURCE_DIR}/lib/librgbmatrix.a)



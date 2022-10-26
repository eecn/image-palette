cmake_minimum_required (VERSION 3.2)

project ("deminant_palette")

aux_source_directory(. DIR_LIB_SRCS)

add_executable(deminant_palette ${DIR_LIB_SRCS})

# test
include_directories(D:\\Build_Libs\\opencv\\build_msvc\\install\\include)

target_link_libraries(deminant_palette  D:\\Build_Libs\\opencv\\build_msvc\\install\\x64\\vc17\\lib\\opencv_world460d.lib)
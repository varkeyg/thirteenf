set(CMAKE_TOOLCHAIN_FILE /home/gvarkey/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")
cmake_minimum_required(VERSION 3.2)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fsanitize=address")
set(mwlib mwlib)
project(mw)

find_package(CURL CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(RocksDB CONFIG REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)
find_package(tsl-ordered-map CONFIG REQUIRED)

add_library(${mwlib} STATIC include/helper.h src/helper.cpp)
target_include_directories(${mwlib} PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>  $<INSTALL_INTERFACE:>)


target_link_libraries(${mwlib} PUBLIC CURL::libcurl)
target_link_libraries(${mwlib} PUBLIC glog::glog)
target_link_libraries(${mwlib} PUBLIC GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)
target_link_libraries(${mwlib} PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(${mwlib} PUBLIC RocksDB::rocksdb)
target_link_libraries(${mwlib} PUBLIC tinyxml2::tinyxml2)
target_link_libraries(${mwlib} PUBLIC tsl::ordered_map)

export(TARGETS ${mwlib} FILE ${mwlib}Targets.cmake)
export(PACKAGE ${mwlib})
install(TARGETS ${mwlib}
   EXPORT ${mwlib}Targets
   LIBRARY DESTINATION lib
   INCLUDES DESTINATION include
)

install(FILES include/helpertools.h DESTINATION include)
install(EXPORT ${mwlib}Targets
   FILE ${mwlib}Config.cmake
   DESTINATION lib/cmake/${mwlib}
)


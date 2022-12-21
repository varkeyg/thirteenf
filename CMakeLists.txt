set(CMAKE_TOOLCHAIN_FILE /Users/gvarkey/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")
cmake_minimum_required(VERSION 3.2)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fsanitize=address")
set(libtf thirteenf)
project(mw)

find_package(CURL CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(RocksDB CONFIG REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)
find_package(tsl-ordered-map CONFIG REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)

add_library(${libtf} STATIC include/helpertools.h src/helpertools.cpp include/thirteenf.h src/thirteenf.cpp)
# target_include_directories(${libtf} PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>  $<INSTALL_INTERFACE:>)
target_include_directories(${libtf} PUBLIC ${CMAKE_SOURCE_DIR}/include)


target_link_libraries(${libtf} PUBLIC CURL::libcurl)
target_link_libraries(${libtf} PUBLIC glog::glog)
target_link_libraries(${libtf} PUBLIC GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)
target_link_libraries(${libtf} PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(${libtf} PUBLIC RocksDB::rocksdb)
target_link_libraries(${libtf} PUBLIC tinyxml2::tinyxml2)
target_link_libraries(${libtf} PUBLIC tsl::ordered_map)
target_link_libraries(${libtf} PUBLIC SQLiteCpp)



# export(TARGETS ${libtf} FILE ${libtf}Targets.cmake)
# export(PACKAGE ${libtf})
# install(TARGETS ${libtf}
#    EXPORT ${libtf}Targets
#    LIBRARY DESTINATION lib
#    INCLUDES DESTINATION include
# )

# install(FILES include/helpertools.h DESTINATION include)
# install(EXPORT ${libtf}Targets
#    FILE ${libtf}Config.cmake
#    DESTINATION lib/cmake/${libtf}
# )

add_executable(thirteenf_test test/thirteenf_test.cpp)
target_include_directories(thirteenf_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(thirteenf_test PUBLIC ${libtf})

add_executable(tfapp src/tfapp.cpp)
target_include_directories(tfapp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tfapp PUBLIC ${libtf})

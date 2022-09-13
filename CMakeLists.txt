cmake_minimum_required(VERSION 3.0)
project(IDC)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
# include(cmake/FindMySQL.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated  -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

find_package(MySQL REQUIRED)
if(MYSQL_FOUND)
    include_directories(${MYSQL_INCLUDE_DIR})
endif()

find_package(Boost REQUIRED)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
endif()

set(LIB_SRC
	src/public/_public.cpp 
	src/mysql/_mysql.cpp
	src/idc/crtdata.cpp
	src/log/log.cpp
	src/config/config.cpp
	src/public/util.cpp
	src/tools/obtcodetodb.cpp
	src/tools/obtdatatodb.cpp
)
add_library(IDC SHARED ${LIB_SRC})

set(LIB_LIB
	IDC
    mysqlclient
	pthread
	yaml-cpp)

add_executable(test_crtdata test/test_crtdata.cpp)
add_dependencies(test_crtdata IDC)
target_link_libraries(test_crtdata ${LIB_LIB})

add_executable(test_mysql test/test_mysql.cpp)
add_dependencies(test_mysql IDC)
target_link_libraries(test_mysql ${LIB_LIB})

add_executable(test_datatodb test/test_datatodb.cpp)
add_dependencies(test_datatodb IDC)
target_link_libraries(test_datatodb ${LIB_LIB})
# add_executable(procctl tools/procctl.cpp)
# add_dependencies(procctl IDC)
# target_link_libraries(procctl ${LIB_LIB})

# add_executable(checkproc tools/checkproc.cpp)
# add_dependencies(checkproc IDC)
# target_link_libraries(checkproc ${LIB_LIB})

# add_executable(createtable tools/createtable.cpp)
# add_dependencies(createtable IDC)
# target_link_libraries(createtable ${LIB_LIB})

# add_executable(inserttable tools/inserttable.cpp)
# add_dependencies(inserttable IDC)
# target_link_libraries(inserttable ${LIB_LIB})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
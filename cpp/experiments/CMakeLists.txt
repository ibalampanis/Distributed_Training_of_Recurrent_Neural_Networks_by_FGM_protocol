cmake_minimum_required(VERSION 3.3.2)
project(Distributed_Training_of_Recurrent_Neural_Networks_by_FGM_protocol)
set(DCMAKE_CXX_COMPILER "g++-7")
set(CMAKE_CXX_STANDARD 14)  # enable C++14 standard
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

option(DEBUG "DEBUG" ON)

set(CPP_SOURCE_DIR ..)
include_directories("../..")
include_directories(${PYTHON_INCLUDE_DIRS})


############################################################################
########################### CentrEEG #######################################
############################################################################

set(MLSOURCES ../models/rnn.cc)

add_executable(CentrLearn centr_training.cc ${MLSOURCES})
target_link_libraries(CentrLearn
        jsoncpp
        ${MLPACK_LIBRARIES}
        ${ARMADILLO_LIBRARIES}
        ${Boost_LIBRARIES}
        )

############################################################################
######################## GMNetEEG / FGMNetEEG ##############################
############################################################################
add_executable(GMLearn gm_training_net.cc)
add_executable(FGMLearn fgm_training_net.cc)

# dml Library Building
# The abbreviation dml came from "Distributed Machine Learning"
#
# This is a static library which has built from the below files:
#       -> models/rnn.cc
#       -> networks/*.cc
#       -> networks/ddssim/*.cc

file(GLOB DDSSIM_SOURCES ../networks/ddsim/*.cc)
file(GLOB NET_SOURCES ../networks/*.cc)
set(ML_SOURCES ../models/rnn.cc)
set(DML_SOURCES ${ML_SOURCES} ${DDSSIM_SOURCES} ${NET_SOURCES})
add_library(dml STATIC ${DML_SOURCES})


include_directories(${CMAKE_BINARY_DIR}/)
file(TOUCH ${CMAKE_BINARY_DIR}/cpp/experiments/libdml.a)
find_library(DML_LIBRARIES dml ${CMAKE_BINARY_DIR}/cpp/experiments)
message(STATUS "Found DML_LIBRARIES: [${DML_LIBRARIES}]")
target_link_libraries(GMLearn
        jsoncpp
        ${DML_LIBRARIES}
        ${HDF5_LIBRARIES}
        ${MLPACK_LIBRARIES}
        ${ARMADILLO_LIBRARIES}
        ${Boost_LIBRARIES}
        ${PYTHON_LIBRARIES}
        )

target_link_libraries(FGMLearn
        jsoncpp
        ${DML_LIBRARIES}
        ${HDF5_LIBRARIES}
        ${MLPACK_LIBRARIES}
        ${ARMADILLO_LIBRARIES}
        ${Boost_LIBRARIES}
        ${PYTHON_LIBRARIES}
        )

############################################################################
########################## Other Settings ##################################
############################################################################

if (DEBUG)
    message("Compilation with debug info (with ggdb3 flag)")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb3")
else ()
    message("Compilation without debug info (without ggdb3 flag)")
endif ()

#Enable OpenMP package
find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()


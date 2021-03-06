project(inovesa)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(./src SRC_LIST)

set(SRC_LIST
  ./src/CL/OpenCLHandler.cpp
  ./src/IO/Display.cpp
  ./src/IO/GUI/GUIElement.cpp
  ./src/IO/GUI/Plot2DLine.cpp
  ./src/IO/GUI/Plot3DColormap.cpp
  ./src/IO/HDF5File.cpp
  ./src/IO/ProgramOptions.cpp
  ./src/PS/ElectricField.cpp
  ./src/PS/PhaseSpace.cpp
  ./src/PS/PhaseSpaceFactory.cpp
  ./src/SM/DriftMap.cpp
  ./src/SM/FokkerPlanckMap.cpp
  ./src/SM/KickMap.cpp
  ./src/SM/RFKickMap.cpp
  ./src/SM/RotationMap.cpp
  ./src/SM/Identity.cpp
  ./src/SM/SourceMap.cpp
  ./src/SM/WakeKickMap.cpp
  ./src/SM/WakePotentialMap.cpp
  ./src/SM/WakeFunctionMap.cpp
  ./src/Z/CollimatorImpedance.cpp
  ./src/Z/ConstImpedance.cpp
  ./src/Z/Impedance.cpp
  ./src/Z/ImpedanceFactory.cpp
  ./src/Z/FreeSpaceCSR.cpp
  ./src/Z/ParallelPlatesCSR.cpp
  ./src/Z/ResistiveWall.cpp
  ./src/MessageStrings.cpp
  ./src/main.cpp
)

set(INC_LIST
  ./inc/Array.h
  ./inc/CL/local_cl.hpp
  ./inc/CL/OpenCLHandler.hpp
  ./inc/SM/DriftMap.hpp
  ./inc/PS/ElectricField.hpp
  ./inc/PS/PhaseSpace.hpp
  ./inc/PS/PhaseSpaceFactory.hpp
  ./inc/PS/Ruler.hpp
  ./inc/IO/Display.hpp
  ./inc/IO/GUI/GUIElement.hpp
  ./inc/IO/GUI/Plot2DLine.hpp
  ./inc/IO/GUI/Plot3DColormap.hpp
  ./inc/IO/HDF5File.hpp
  ./inc/IO/ProgramOptions.hpp
  ./inc/SM/FokkerPlanckMap.hpp
  ./inc/SM/KickMap.hpp
  ./inc/SM/RFKickMap.hpp
  ./inc/SM/Identity.hpp
  ./inc/SM/RotationMap.hpp
  ./inc/SM/SourceMap.hpp
  ./inc/SM/WakePotentialMap.hpp
  ./inc/SM/WakeKickMap.hpp
  ./inc/SM/WakeFunctionMap.hpp
  ./inc/Z/CollimatorImpedance.hpp
  ./inc/Z/ConstImpedance.hpp
  ./inc/Z/Impedance.hpp
  ./inc/Z/ImpedanceFactory.hpp
  ./inc/Z/FreeSpaceCSR.hpp
  ./inc/Z/ParallelPlatesCSR.hpp
  ./inc/Z/ResistiveWall.hpp
  ./inc/MessageStrings.hpp
  ./inc/defines.hpp
  ./inc/fixed_point.h
)

add_executable(${PROJECT_NAME} ${SRC_LIST} ${INC_LIST} ${MISC_LIST})

include(CheckIncludeFileCXX)

execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions("-DGIT_BRANCH=\"${GIT_BRANCH}\"")

execute_process(
  COMMAND git rev-parse --verify HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions("-DGIT_COMMIT=\"${GIT_COMMIT}\"")

add_definitions(-std=c++14)
if(DEFINED CMAKE_BUILD_TYPE)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#        if ( CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#            add_definitions(-Weverything -Wno-padded -Wno-c++98-compat -Wno-c++98-compat-pedantic)
#        else()
            add_definitions(-Wall -Wextra)
#        endif()
        add_definitions(-g -DDEBUG)
        MESSAGE ("Doing debug compile, including debug symbols.")
    else()
        add_definitions(-Wall -O3 -w )
        MESSAGE ("Doing release compile, using optimization flags.")
    endif()
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "./cmake")
find_package(OpenCL QUIET)
IF(OPENCL_FOUND)
    add_definitions( -DINOVESA_USE_CL)
    MESSAGE ("Found OpenCL. Will add support.")
    find_package(clFFT QUIET)
    IF(clFFT_FOUND)
        add_definitions( -DINOVESA_USE_CLFFT)
        MESSAGE ("Found clFFT. Will add support.")
    ENDIF()
ELSE()
    MESSAGE ("Did not find OpenCL. Will compile without OpenCL support.")
ENDIF()
find_package(OpenGL QUIET)
IF(OPENGL_FOUND)
    add_definitions( -DINOVESA_USE_GUI)
    MESSAGE ("Found OpenGL. Will add support.")
    find_package(GLEW REQUIRED QUIET)
    find_package(GLFW REQUIRED QUIET)
    add_definitions( -DGLFW_VERSION_MAJOR=${GLFW_VERSION_MAJOR})
ELSE()
    MESSAGE ("Did not find OpenGL. Will compile without OpenGL support.")
ENDIF()
find_package(Boost COMPONENTS filesystem program_options system REQUIRED QUIET)
find_package(FFTW REQUIRED QUIET)
find_package(HDF5 COMPONENTS CXX QUIET)
IF(HDF5_FOUND)
    add_definitions( -DINOVESA_USE_HDF5)
    MESSAGE ("Found HDF5. Will add support.")
    set (INOVESA_HDF5_INCLUDE_DIRS ${HDF5_INCLUDE_DIRS})
    set (INOVESA_HDF5_LIBRARIES ${HDF5_CXX_LIBRARIES} ${HDF5_LIBRARIES})
ELSE()
    MESSAGE ("Did not find HDF5. Will compile without HDF5 support.")
ENDIF()
find_package(PNG++ QUIET)
IF(PNG++_FOUND)
    add_definitions( -DINOVESA_USE_PNG)
    MESSAGE ("Found PNG++. Will add PNG support.")
ELSE()
    MESSAGE ("Did not find PNG++. Will compile without PNG support.")
ENDIF()

include_directories(./inc
    ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS}
    ${INOVESA_HDF5_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS} ${FFTW_INCLUDE_DIRS}
    ${OPENCL_INCLUDE_DIRS} ${CLFFT_INCLUDE_DIRS}
)
SET(LIBS ${LIBS} ${Boost_LIBRARIES}
    ${GLFW_LIBRARIES} ${GLEW_LIBRARIES}
    ${GL_LIBRARY} ${INOVESA_HDF5_LIBRARIES}
    ${OPENGL_LIBRARIES} ${OPENCL_LIBRARIES}
    ${PNG_LIBRARIES} ${FFTW_LIBRARIES}
    ${CLFFT_LIBRARIES}
)
target_link_libraries(${PROJECT_NAME} ${LIBS})

set(Python_ADDITIONAL_VERSIONS 2.7 2.6)

find_package(PythonLibs REQUIRED)
find_package(SWIG REQUIRED)

message (STATUS "Found Python headers: ${PYTHON_INCLUDE_PATH}")

include(${SWIG_USE_FILE})

include_directories(${DETECTOR_INCLUDE_DIR}
                    ${EVENT_INCLUDE_DIR}
                    ${UTILITIES_INCLUDE_DIR}
                    ${SIMULATION_INCLUDE_DIR}
                    ${TINYXML_INCLUDE_DIR}
                    ${FFTW3_INCLUDE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${PYTHON_INCLUDE_PATH})

# Disable -Wshadow for SWIG generated code
string(REPLACE "-Wshadow" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

# Common SWIG definitions (only imported from other input files)
set_source_files_properties(common.i PROPERTIES CPLUSPLUS ON)

# Event
set_source_files_properties(event.i PROPERTIES CPLUSPLUS ON)
swig_add_module(event python event.i common.i ${EVENT_SOURCES})
swig_link_libraries(event Event Detector Util ${PYTHON_LIBRARIES})

# Detector
set_source_files_properties(detector.i PROPERTIES CPLUSPLUS ON)
swig_add_module(detector python detector.i common.i ${DETECTOR_SOURCES})
swig_link_libraries(detector Detector Event Util ${PYTHON_LIBRARIES})

# Utilities
set_source_files_properties(utilities.i PROPERTIES CPLUSPLUS ON)
swig_add_module(utilities python utilities.i common.i ${UTILITIES_SOURCES})
swig_link_libraries(utilities Util tinyxml ${PYTHON_LIBRARIES})

# Simulation
set_source_files_properties(simulation.i PROPERTIES CPLUSPLUS ON)
swig_add_module(simulation python simulation.i common.i ${SIMULATION_SOURCES})
swig_link_libraries(simulation Sim ${PYTHON_LIBRARIES})

install(FILES __init__.py
              ${CMAKE_CURRENT_BINARY_DIR}/event.py
              ${CMAKE_CURRENT_BINARY_DIR}/detector.py
              ${CMAKE_CURRENT_BINARY_DIR}/utilities.py
              ${CMAKE_CURRENT_BINARY_DIR}/simulation.py
        DESTINATION lib/mwoffline)

install(TARGETS ${SWIG_MODULE_event_REAL_NAME}
                ${SWIG_MODULE_detector_REAL_NAME}
                ${SWIG_MODULE_utilities_REAL_NAME}
                ${SWIG_MODULE_simulation_REAL_NAME}
        LIBRARY
        DESTINATION lib/mwoffline)

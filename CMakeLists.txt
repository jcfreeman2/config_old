tdaq_package()

remove_definitions(-DERS_NO_DEBUG)

tdaq_add_library(config src/*.cpp
                 INCLUDE_DIRECTORIES PRIVATE Boost
                 LINK_LIBRARIES tdaq-common::ers)

tdaq_add_executable(config_dump bin/config_dump.cpp                    LINK_LIBRARIES config Boost::program_options)
tdaq_add_executable(config_export_data bin/config_export_data.cpp      LINK_LIBRARIES config Boost::program_options)
tdaq_add_executable(config_export_schema bin/config_export_schema.cpp  LINK_LIBRARIES config Boost::program_options)

tdaq_add_executable(config_time_test test/config_time_test.cpp         LINK_LIBRARIES config)
tdaq_add_executable(config_test_object test/config_test_object.cpp     LINK_LIBRARIES config)
tdaq_add_executable(config_test_rw test/config_test_rw.cpp             LINK_LIBRARIES config)

tdaq_add_library(pyconfig src/python/config.cpp INCLUDE_DIRECTORIES PythonLibs LINK_LIBRARIES PRIVATE config Boost::python)

target_compile_options(pyconfig PRIVATE -Wno-unused) 

tdaq_add_python_package(config)

tdaq_add_jar(jconfig jsrc/config/*.java INCLUDE_JARS Jers/ers.jar TDAQExtJars/external.jar OUTPUT_NAME config)

add_test(NAME config_check_cpp  COMMAND ${TDAQ_RUNNER} ${CMAKE_CURRENT_SOURCE_DIR}/test/test.sh ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_test(NAME config_check_py   COMMAND ${TDAQ_RUNNER} ${CMAKE_CURRENT_SOURCE_DIR}/test/pytest.sh ${CMAKE_CURRENT_SOURCE_DIR})

tdaq_add_test(NAME PyStyleTest POST_INSTALL COMMAND env PYTHONDONTWRITEBYTECODE=x ${CMAKE_CURRENT_SOURCE_DIR}/test/pystyle.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

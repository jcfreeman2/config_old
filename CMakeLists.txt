cmake_minimum_required(VERSION 3.12)
project(config VERSION 0.0.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(Boost COMPONENTS program_options REQUIRED)
find_package(ers REQUIRED)
find_package(logging REQUIRED)

daq_add_library(*.cpp LINK_LIBRARIES ers::ers logging::logging)

daq_add_application(config_dump config_dump.cpp                    LINK_LIBRARIES config Boost::program_options)
daq_add_application(config_export_data config_export_data.cpp      LINK_LIBRARIES config Boost::program_options)
daq_add_application(config_export_schema config_export_schema.cpp  LINK_LIBRARIES config Boost::program_options)

daq_add_application(config_time_test config_time_test.cpp     TEST    LINK_LIBRARIES config)
daq_add_application(config_test_object config_test_object.cpp TEST    LINK_LIBRARIES config)
daq_add_application(config_test_rw config_test_rw.cpp         TEST    LINK_LIBRARIES config)

# JCF, Oct-18-2022: have yet to handle the creation of pyconfig
#tdaq_add_library(pyconfig src/python/config.cpp INCLUDE_DIRECTORIES PythonLibs LINK_LIBRARIES PRIVATE config Boost::python)

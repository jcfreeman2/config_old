/**
 * @file config/src/python/embedding.cpp
 * @author<a href="mailto:andre.dos.anjos@cern.ch">Andre Anjos</a>
 *
 * @brief Example python/C++ Configuration embedded system
 */

// This example will create a Configuration object using a file and then
// call the python interpreter passing that Configuration object. In python,
// the Configuration object will be used for displaying which objects are
// available in the database.

#include <Python.h>
#include <boost/python.hpp>
#include <iostream>
#include <cstdlib>
#include <memory>
#include "config/ConfigurationPointer.h"

int main(int argc, char** argv) {
  using namespace boost::python;
  
  if (argc == 1) {
    std::cout << "usage: " << argv[0] << " <oks-database.data.xml> <script.py> [script-arguments]"
      << std::endl;
    std::exit(1);
  }
  
  // Database to open is a bit more complicated...
  std::string connection = "oksconfig:" + std::string(argv[1]);

  // We open the database. Normally in the DBE this is created when the user
  // starts the GUI.
  Configuration confdb(connection);

  std::cout << "[c++] Connection to database " << connection 
    << " established." << std::endl;
  
  // We start the interpreter main module.
  Py_Initialize();

  // Normally, the user will have a script and we are going to run it. For this
  // simple example, we are going to define the following:
  //
  // 1) Only one fixed parameter is "passed" into python, being this the
  // Configuration object opened earlier. That will be bound to a python object
  // with the name "database". The user script must know that. This object will
  // be in the global scope and represents the pythonic configuration object
  // that the user manipulates.
  //
  // 2) If the user wants to print output, it will go to what in C++ is bound to
  // std::out. For the DBE, one may be willing to diverge that into a window for
  // the script log. Please check the Python C API on a way to do this.
  //
  // To pass more parameters the DBE maintainer has to define a way for the user
  // to specify what it wants as input. An example would be an XML or python
  // file that defines how to map input (from the user) as a set of strings that
  // is passed to the script using PySys_SetArgv(). Read that function's
  // documentation to understand how to use it. In this example we take the user
  // input from the remaining of the command line, so nothing really sexy.
  PySys_SetArgv(argc-2, &argv[2]);

  // Create the boost python Configuration representation
  // Insert the "database" object into the __main__ namespace (global)
  
  // Needless to say, we put all  in a try/catch block, to prevent the DBE to
  // potentially crash evaluating user scripts.
  try {
    object main_module = import("__main__");
    object main_namespace = main_module.attr("__dict__");
    main_namespace["pm"] = import("pm");
    main_namespace["pm.project"] = import("pm.project");
    main_namespace["database"] = object(python::ConfigurationPointer(confdb));

    //run the user script
    std::cout << "[c++] Starting user script " << argv[2] << "..." << std::endl;
    exec("database = pm.project.Project(database)", 
         main_namespace, main_namespace);
    exec_file(argv[2], main_namespace, main_namespace);
    std::cout << "[c++] Finished executing user script." << std::endl;

  } catch(error_already_set const &) {
    PyErr_Print(); //in the DBE, diverge this to a window... 
  } catch(std::exception const & e) {
    std::cerr << "std::exception thrown: " << e.what() << std::endl;
  } catch(...) {
    std::cerr << "Unknonw exception thrown." << std::endl;
  }

  //DO NOT CALL Py_Finalize() as Boost::Python currently does not support this
  //Py_Finalize();
}

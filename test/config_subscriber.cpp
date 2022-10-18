//
//  FILE: src/dal_notify.cpp
//
//  Example program which shows how to subscribe on application changes
//  and receive notification. It also demonstrates how to switch between
//  OKS and RDB implementations.
//
//  For command line arguments see function usage() or run the program
//  with --help.
//
//  Implementation:
//	<Igor.Soloviev@cern.ch> - June 2003
//
//  Modified:
//

#include <signal.h>
#include <unistd.h>


  // include headers describing abstract configuration interface

#include "config/Change.h"
#include "config/ConfigObject.h"
#include "config/Configuration.h"

  /**
   *  The callback function is called when changes occurred.
   *  It contains all changes.
   */

void
cb1(const std::vector<ConfigurationChange *> & changes, void * parameter)
{
  Configuration * configuration = reinterpret_cast<Configuration *>(parameter);

  std::cout << "CALLBACK 1 (report all changes):\n";

    // iterate changes sorted by classes

  for(auto& j : changes) {

    // get class name
    std::cout << "- there are changes in class \"" << j->get_class_name() << "\"\n";

    // print class name
    for(auto& i : j->get_modified_objs()) {
      std::cout << "  * object \""  << i << "\" was modified\n";
      ConfigObject obj;
      configuration->get(j->get_class_name(),i,obj);
      obj.print_ref(std::cout, *configuration, "  ");
    }

    // print removed objects
    for(auto& i : j->get_removed_objs()) {
      std::cout << "  * object \""  << i << "\" was removed\n";
    }

    // print created objects
    for(auto& i : j->get_created_objs()) {
      std::cout << "  * object \""  << i << "\" was created\n";
      ConfigObject obj;
      configuration->get(j->get_class_name(),i,obj);
      obj.print_ref(std::cout, *configuration, "  ");
    }
  }

  std::cout.flush();
}


  /**
   *  Function to print out how to use the program.
   */

static void
usage()
{
  std::cout <<
    "Usage: config_subscriber [-d | --data database-name]\n"
    "                         [-c | --classes [class-1 ...] ]\n"
    "\n"
    "Options/Arguments:\n"
    "       -d database-name  name of the database n format \"plugin:params\" (ignore TDAQ_DB variable)\n"
    "       -c                subscribe on any changes in listed classes\n"
    "\n"
    "Description:\n"
    "       Example of selective subscription on changes.\n";
}


extern "C" void signal_handler(int sig)
{
  std::cout << "config_subscriber caught signal " << sig << std::endl;
}


  /**
   *  The main function.
   */

int main(int argc, char *argv[])
{
    // parse command line

  std::string db_name;

  std::list<std::string> classes;


    // parse command line parameters

  {
    for(int i = 1; i < argc; i++) {
      if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
        usage();
        return (EXIT_SUCCESS);
      }
      else if(!strcmp(argv[i], "-d") || !strcmp(argv[i], "--data")) {
        if(++i == argc) {
          std::cerr << "ERROR: no data file provided\n\n";
          return (EXIT_FAILURE);
	}
	else {
          db_name = argv[i];
	}
      }
      else if(!strcmp(argv[i], "-c") || !strcmp(argv[i], "--classes")) {
        if(++i == argc) {
          std::cerr << "ERROR: no name provided\n\n";
          return (EXIT_FAILURE);
        }
        else {
            int j = 0;
            for(; j < argc - i - 1; ++j) {
              if(argv[i+1+j][0] != '-') classes.push_back(argv[i+1+j]);
              else {
                break;
              }
            }
            i += j;
        }
      }
      else {
        std::cerr << "ERROR: Unexpected parameter: \"" << argv[i] << "\"\n\n";
        usage();
        return (EXIT_FAILURE);
      }
    }
  }

  try {
    Configuration conf(db_name);

    signal(SIGINT,signal_handler);
    signal(SIGTERM,signal_handler);


    if(classes.empty())
      {
        ::ConfigurationSubscriptionCriteria c;
        conf.subscribe(c, cb1, reinterpret_cast<void *>(&conf));
      }
    else
      {
        for(auto& x : classes)
          {
            ::ConfigurationSubscriptionCriteria c;
            c.add(x);
            conf.subscribe(c, cb1, reinterpret_cast<void *>(&conf));
          }
      }

    try {
      pause();
    }
    catch (...) {
      ;
    }

    std::cout << "Exiting config_subscriber ..." << std::endl;

    conf.unsubscribe();
  }
  catch (daq::config::Exception & ex) {
    std::cerr << "Caught " << ex << std::endl;
    return (EXIT_FAILURE);
  }

  return 0;
}

#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>

#include <chrono>
#include <iostream>
#include <fstream>
#include <string>

#include <config/Configuration.h>
#include <config/ConfigObject.h>
#include <config/Schema.h>

ERS_DECLARE_ISSUE(
  config_time_test,
  BadCommandLine,
  "bad command line: " << reason,
  ((const char*)reason)
)

ERS_DECLARE_ISSUE(
  config_time_test,
  ConfigException,
  "caught daq::config::Exception exception",
)

static void
no_param(const char * s)
{
  std::ostringstream text;
  text << "no parameter for " << s << " provided";
  ers::fatal(config_time_test::BadCommandLine(ERS_HERE, text.str().c_str()));
  exit(EXIT_FAILURE);
}

template <class T>
void
stop_and_report(T& tp, const char * fname)
{
  std::cout << "TEST \"" << fname << "\" => " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-tp).count() / 1000. << " ms\n";
}


int main(int argc, char *argv[])
{
  const char * db_name = 0;
  bool verbose = false;

  for(int i = 1; i < argc; i++) {
    const char * cp = argv[i];

    if(!strcmp(cp, "-h") || !strcmp(cp, "--help")) {
      std::cout << 
        "Usage: config_time_test -d dbspec [-c | -C [class_name]] [-o | -O [object_id]] [-n]\n"
        "\n"
        "Options/Arguments:\n"
        "  -d | --database dbspec        database specification in format plugin-name:parameters\n"
        "  -v | --verbose                print details\n"
        "\n"
        "Description:\n"
        "  The utility reports results of time tests.\n"
        "  When no -c or -o options are provided, utility lists all classes.\n\n";
    }
    else if(!strcmp(cp, "-d") || !strcmp(cp, "--database")) {
      if(++i == argc) { no_param(cp); } else { db_name = argv[i]; }
    }
    else if(!strcmp(cp, "-v") || !strcmp(cp, "--verbose")) {
      verbose = true;
    }
  }

  if(!db_name) {
    ers::fatal(config_time_test::BadCommandLine(ERS_HERE, "no database name given"));
    return (EXIT_FAILURE);
  }

  try {
  
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    auto tp = std::chrono::steady_clock::now();

    Configuration conf(db_name);

    if(verbose) {
      std::cout << "load database \"" << conf.get_impl_spec() << '\"' << std::endl;
    }

    stop_and_report(tp, "loading database");

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    tp = std::chrono::steady_clock::now();

    std::set<std::string> classes;

    for(config::fmap<config::fset>::const_iterator i = conf.superclasses().begin(); i != conf.superclasses().end(); ++i) {
      classes.insert(*i->first);
    }

    if(verbose) {
      std::cout << "The database schema has " << classes.size() << " class(es):\n";
    }

    for(std::set<std::string>::const_iterator i = classes.begin(); i != classes.end(); ++i) {
      const daq::config::class_t& d(conf.get_class_info(*i));
      if(verbose) {
        d.print(std::cout, "  ");
      }
    }

    stop_and_report(tp, "reading schema meta-information");

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    tp = std::chrono::steady_clock::now();

    std::vector<ConfigObject> all_objects;

    for(std::set<std::string>::const_iterator i = classes.begin(); i != classes.end(); ++i) {
      std::vector<ConfigObject> objects;
      conf.get(*i, objects);

      unsigned int count = 0;

      for(std::vector<ConfigObject>::const_iterator j = objects.begin(); j != objects.end(); ++j) {
        if((*j).class_name() == *i) {
	  count++;
	  all_objects.push_back(*j);
	}
      }
      
      if(verbose) {
        std::cout << "Class " << *i << " has " << count << " objects (" << objects.size() << " with derived classes)\n";
      }
    }
    
    if(verbose) {
      std::cout << "Total number of objects: " << all_objects.size() << std::endl;
    }

    stop_and_report(tp, "reading names of objects");

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    tp = std::chrono::steady_clock::now();

    std::set<std::string> files;
    
    for(std::vector<ConfigObject>::const_iterator i = all_objects.begin(); i != all_objects.end(); ++i) {
      files.insert((*i).contained_in());
    }
    
    if(verbose) {
      std::cout << "There are " << files.size() << " data files:\n";
      
      for(std::set<std::string>::const_iterator i = files.begin(); i != files.end(); ++i) {
        std::cout << " - \"" << *i << "\"\n";
      }
    }
    
    stop_and_report(tp, "reading names of files");

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    tp = std::chrono::steady_clock::now();

    if(verbose) {
      std::cout << "Details of objects:\n";

      for(std::vector<ConfigObject>::const_iterator i = all_objects.begin(); i != all_objects.end(); ++i) {
        (*i).print_ref(std::cout, conf, "  ");
      }
    }
    else {
      std::ofstream null("/dev/null", std::ios::out);
      for(std::vector<ConfigObject>::const_iterator i = all_objects.begin(); i != all_objects.end(); ++i) {
        (*i).print_ref(null, conf);
      }
    }

    stop_and_report(tp, "reading all attributes and relationships");

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    return 0;
  }
  catch (daq::config::Exception & ex) {
    ers::fatal(config_time_test::ConfigException(ERS_HERE, ex));
  }

  return (EXIT_FAILURE);
}

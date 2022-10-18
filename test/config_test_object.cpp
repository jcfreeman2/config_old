#include <stdlib.h>

#include <iostream>
#include <string>

#include <config/Configuration.h>
#include <config/ConfigObject.h>

ERS_DECLARE_ISSUE(
  config_test_object,
  BadCommandLine,
  "bad command line: " << reason,
  ((const char*)reason)
)

ERS_DECLARE_ISSUE(
  config_test_object,
  ConfigException,
  "caught daq::config::Exception exception",
)

static void
usage()
{
  std::cout << 
    "Usage: config_test_object -d | --database dbspec\n"
    "                          -c | --class-name class\n"
    "                          -o | --object-id object\n"
    "\n"
    "Options/Arguments:\n"
    "       -d dbspec    database specification in format plugin-name:parameters\n"
    "       -c class     name of the class to dump\n"
    "       -o object    optional id of the object to dump\n"
    "\n"
    "Description:\n"
    "       The utility tests object existence.\n\n";

}

static void
no_param(const char * s)
{
  std::ostringstream text;
  text << "no parameter for " << s << " provided";
  ers::fatal(config_test_object::BadCommandLine(ERS_HERE, text.str().c_str()));
  exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{
  const char * db_name = 0;
  const char * class_name = 0;
  const char * object_id = 0;

  for(int i = 1; i < argc; i++) {
    const char * cp = argv[i];

    if(!strcmp(cp, "-h") || !strcmp(cp, "--help")) {
      usage();
      return 0;
    }
    else if(!strcmp(cp, "-d") || !strcmp(cp, "--database")) {
      if(++i == argc) { no_param(cp); } else { db_name = argv[i]; }
    }
    else if(!strcmp(cp, "-c") || !strcmp(cp, "--class-name")) {
      if(++i == argc) { no_param(cp); } else { class_name = argv[i]; }
    }
    else if(!strcmp(cp, "-o") || !strcmp(cp, "--object-id")) {
      if(++i == argc) { no_param(cp); } else { object_id = argv[i]; }
    }
    else {
      std::ostringstream text;
      text << "unexpected parameter: \'" << cp << "\'; run command with --help to see valid command line options.";
      ers::fatal(config_test_object::BadCommandLine(ERS_HERE, text.str().c_str()));
      return (EXIT_FAILURE);
    }
  }

  if(!db_name) {
    ers::fatal(config_test_object::BadCommandLine(ERS_HERE, "no database name given"));
    return (EXIT_FAILURE);
  }

  if(!class_name) {
    ers::fatal(config_test_object::BadCommandLine(ERS_HERE, "no class name given"));
    return (EXIT_FAILURE);
  }

  if(!object_id) {
    ers::fatal(config_test_object::BadCommandLine(ERS_HERE, "no object id given"));
    return (EXIT_FAILURE);
  }

  try {
    Configuration db(db_name);

    if(db.test_object(class_name, object_id)) {
      std::cout << "object \'" << object_id << '@' << class_name << "\' exists" << std::endl;
    }
    else {
      std::cout << "object \'" << object_id << '@' << class_name << "\' does not exist" << std::endl;
    }

    return 0;
  }
  catch (daq::config::Exception & ex) {
    ers::fatal(config_test_object::ConfigException(ERS_HERE, ex));
  }

  return (EXIT_FAILURE);
}

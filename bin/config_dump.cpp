#include <stdlib.h>

#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include <config/Configuration.h>
#include <config/ConfigObject.h>
#include <config/Schema.h>

ERS_DECLARE_ISSUE(
  config_dump,
  BadCommandLine,
  "bad command line: " << reason,
  ((const char*)reason)
)

ERS_DECLARE_ISSUE(
  config_dump,
  ConfigException,
  "caught daq::config::Exception exception",
)

struct SortByName
{
  bool
  operator()(const ConfigObject *o1, const ConfigObject *o2) const
  {
    return o1->UID() < o2->UID();
  }
};

static void
print_referenced_by(const ConfigObject &obj, const char *prefix)
{
  std::vector<ConfigObject> values;
  obj.referenced_by(values, "*", false, 0, 0);
  if (values.size() == 0)
    {
      std::cout << prefix << "is not referenced by others objects\n";
    }
  else
    {
      std::cout << prefix << "is referenced by " << values.size() << " object" << (values.size() == 1 ? "" : "s") << ":\n";
      for (const auto &iobj : values)
        std::cout << prefix << " * " << iobj << std::endl;

    }
}

static void
print_versions(const std::vector<daq::config::Version>& versions)
{
  const auto len = versions.size();
  unsigned int idx = 1;
  for (const auto& x : versions)
    {
      char buf[50];
      std::time_t t(x.get_timestamp());
      std::strftime(buf, 50, "%F %T %Z", std::localtime(&t));
      std::cout << " * version [" << idx++ << '/' << len << "]\n"
          "    id: " << x.get_id() << "\n"
          "    user: " << x.get_user() << "\n"
          "    date: " << buf << "\n"
          "    comment: " << x.get_comment() << "\n"
          "    files:\n";

      for (const auto& f : x.get_files())
        std::cout << "     - \"" << f << "\"\n";
    }
}

int main(int argc, char *argv[])
{
  const std::string any("*");
  std::string db_name, class_name, object_id, since, until;

  bool changes = false;

  bool skip_irrelevant = true;
  daq::config::Version::QueryType query_type = daq::config::Version::query_by_tag;

  bool direct_info = false;
  bool objects_details = false;
  bool contained_in = false;
  bool referenced_by = false;

  boost::program_options::options_description desc(
      "Dumps class and objects descriptions using abstract config API.\n"
      "Without -c or -o options, the utility lists all classes.\n"
      "\n"
      "Options/Arguments");

  try
    {
      std::vector<std::string> vesrions_str;
      std::string class_name2;

      desc.add_options()
        (
          "database,d",
          boost::program_options::value<std::string>(&db_name)->required(),
          "database specification in format plugin-name:parameters"
        )
        (
          "changes,a",
          "print details of new repository versions or modified files"
        )
        (
          "versions,v",
          boost::program_options::value<std::vector<std::string>>(&vesrions_str)->multitoken()->zero_tokens(),
          "print details of versions from archive providing 4 parameters \"all|skip\" \"date|id|tag\" \"since\" \"until\""
        )
        (
          "class-direct-info,c",
          boost::program_options::value<std::string>(&class_name)->default_value("")->implicit_value(any),
          "print direct properties of all classes, or given class if name is provided"
        )
        (
          "class-all-info,C",
          boost::program_options::value<std::string>(&class_name2)->default_value("")->implicit_value(any),
          "similar to -c, but prints all properties of class (all attributes, all superclasses, etc.)"
        )
        (
          "list-objects,o",
          "list objects of class"
        )
        (
          "print-referenced-by,r",
          "print objects referencing given object (only with -o option)"
        )
        (
          "dump-objects,O",
          boost::program_options::value<std::string>(&object_id)->default_value("")->implicit_value(any),
          "dump all objects of class or details of given object, if id is provided (-c is required)"
        )
        (
          "show-contained-in,n",
          "when dump an object, print out the database file it belongs to"
        )
        (
          "help,h",
          "print help message"
        );

      boost::program_options::variables_map vm;
      boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);

      if (vm.count("help"))
        {
          std::cout << desc << std::endl;
          return EXIT_SUCCESS;
        }

      boost::program_options::notify(vm);

      if (!class_name.empty())
        direct_info = true;

      if (!class_name2.empty())
        {
          if (!class_name.empty())
            throw std::runtime_error("cannot use -c and -C options simultaneously");
          else
            class_name = std::move(class_name2);
        }

      if (vm.count("changes"))
        changes = true;

      if (!vesrions_str.empty())
        {
          if (vesrions_str.size() != 4)
            {
              throw std::runtime_error("-v option must have 4 parameters, see help");
            }
          else
            {
              if (vesrions_str[0] == "all")
                skip_irrelevant = false;
              else if (vesrions_str[0] != "skip")
                throw std::runtime_error("first parameter of -v has to be \"all\" or \"skip\"");

              if (vesrions_str[1] == "date")
                query_type = daq::config::Version::query_by_date;
              else if (vesrions_str[1] == "id")
                query_type = daq::config::Version::query_by_id;
              else if (vesrions_str[1] != "tag")
                throw std::runtime_error("second versions parameter must be \"date\", \"id\" or \"tag\"");

              since = vesrions_str[2];
              until = vesrions_str[3];
            }
        }

      if (!object_id.empty())
        objects_details = true;

      if (vm.count("list-objects"))
        {
          objects_details = false;
          object_id = any;
        }

      if (vm.count("print-referenced-by"))
        referenced_by = true;

      if (vm.count("show-contained-in"))
        contained_in = true;

      if (class_name.empty() && !object_id.empty() && object_id != any)
        throw std::runtime_error("object id is set, but no class name given (use -c option)");
    }
  catch (std::exception &ex)
    {
      ers::fatal(config_dump::BadCommandLine(ERS_HERE, ex.what()));
      return EXIT_FAILURE;
    }


  try
    {
      Configuration conf(db_name);

      // get versions if any
      if (changes)
        {
          std::cout << "Changes:\n";
          print_versions(conf.get_changes());
          return EXIT_SUCCESS;
        }

      if (!since.empty())
        {
          std::cout << "Versions:\n";
          print_versions(conf.get_versions(since, until, query_type, skip_irrelevant));
          return EXIT_SUCCESS;
        }

      std::set<std::string> classes;

      if (!class_name.empty() && class_name != any)
        classes.insert(class_name);
      else
        for (const auto &i : conf.superclasses())
          classes.insert(*i.first);

      // if there are no options, just list classes
      if (class_name.empty() && object_id.empty())
        {
          std::cout << "The database schema has " << classes.size() << " class(es):\n";
          for (const auto &i : classes)
            std::cout << " - \'" << i << "\'\n";
          return EXIT_SUCCESS;
        }

      // only print details of classes
      if (!class_name.empty() && object_id.empty())
        {
          if (class_name == any)
            std::cout << "The database schema has " << classes.size() << " class(es):\n";

          for (const auto &i : classes)
            conf.get_class_info(i, direct_info).print(std::cout, "  ");

          return EXIT_SUCCESS;
        }

      const char *prefix = "";
      const char *prefix2 = "  ";
      const char *prefix3 = "    ";

      // list or print all objects of class(es)
      if (object_id == any)
        {
          if (class_name.empty() || class_name == any)
            {
              std::cout << "The database schema has " << classes.size() << " class(es):\n";
              prefix = "  ";
              prefix2 = "    ";
              prefix3 = "      ";
            }

          for (std::set<std::string>::const_iterator i = classes.begin(); i != classes.end(); ++i)
            {
              std::vector<ConfigObject> objects;
              conf.get(*i, objects);
              if (objects.empty())
                {
                  std::cout << prefix << "The class \'" << *i << "\' has no objects\n";
                }
              else
                {
                  std::cout << prefix << "The class \'" << *i << "\' has " << objects.size() << " object(s) including sub-classes:\n";

                  // sort by ID for consistent output
                  std::set<const ConfigObject*, SortByName> sorted_by_id;

                  for (const auto &j : objects)
                    sorted_by_id.insert(&j);

                  for (const auto &j : sorted_by_id)
                    if (j->class_name() == *i)
                      {
                        if (objects_details)
                          j->print_ref(std::cout, conf, prefix2, contained_in);
                        else
                          std::cout << prefix << " - \'" << j->UID() << "\'\n";

                        if (referenced_by)
                          print_referenced_by(*j, prefix3);
                      }
                    else
                      {
                        std::cout << prefix << " - skip \'" << j->UID() << "\' (database class name = \'" << j->class_name() << "\')\n";
                      }
                }
            }
        }
      else
        {
          ConfigObject obj;
          conf.get(class_name, object_id, obj);
          obj.print_ref(std::cout, conf, "", contained_in);
          if (referenced_by)
            print_referenced_by(obj, prefix2);
        }
    }
  catch (daq::config::Exception &ex)
    {
      ers::fatal(config_dump::ConfigException(ERS_HERE, ex));
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

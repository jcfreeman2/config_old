#include <stdlib.h>

#include <iostream>

#include <boost/program_options.hpp>

#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <config/Configuration.h>


int
main(int argc, char *argv[])
{
  std::string output_file, db_name, classes, format("json");
  bool direct_only(false);

  boost::program_options::options_description desc("Export config schema using boost property tree.\n\nOptions/Arguments");

  try
    {
      desc.add_options()
        (
          "database,d",
          boost::program_options::value<std::string>(&db_name)->required(),
          "database specification in format plugin-name:parameters"
        )
        (
          "classes,c",
          boost::program_options::value<std::string>(&classes),
          "regex defining class names; ignore if empty"
        )
        (
          "direct-only,r",
          "print direct properties"
        )
        (
          "output,o",
          boost::program_options::value<std::string>(&output_file),
          "output file name; print to standard out, if not defined"
        )
        (
          "format,t",
          boost::program_options::value<std::string>(&format)->default_value(format),
          "output format (\"json\", \"xml\" or \"info\")"
        )
        (
          "help,h",
          "Print help message"
        );

      boost::program_options::variables_map vm;
      boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);

      if (vm.count("help"))
        {
          std::cout << desc << std::endl;
          return EXIT_SUCCESS;
        }

      if (vm.count("direct-only"))
        direct_only = true;

      boost::program_options::notify(vm);

      auto valid_formats = {"json", "xml", "info"};
      if (std::none_of(valid_formats.begin(), valid_formats.end(), [&format](auto p){ return p == format; }))
        throw std::runtime_error("unsupported format \"" + format + '\"');
    }
  catch (std::exception& ex)
    {
      std::cerr << "command line error: " << ex.what() << std::endl;
      return EXIT_FAILURE;
    }

  try
    {
      Configuration db(db_name);

      boost::property_tree::ptree pt;

      db.export_schema(pt, classes, direct_only);

      std::ostringstream buf;
      if (format == "json")
        boost::property_tree::json_parser::write_json(buf, pt);
      else if (format == "xml")
        boost::property_tree::xml_parser::write_xml(buf, pt, boost::property_tree::xml_writer_make_settings<std::string>(' ', 4));
      else
        boost::property_tree::info_parser::write_info(buf, pt, boost::property_tree::info_writer_make_settings(' ', 4));

      if (!output_file.empty())
        {
          std::ofstream f(output_file);
          f.exceptions(std::ifstream::failbit | std::ifstream::badbit);
          f << buf.str();
          f.close();
        }
      else
        std::cout << buf.str();

      return EXIT_SUCCESS;
    }
  catch (const daq::config::Exception &ex)
    {
      std::cout << "config error: " << ex << std::endl;
    }
  catch (const boost::property_tree::json_parser_error &ex)
    {
      std::cout << "ptree json error: " << ex.what() << std::endl;
    }
  catch (const std::exception &ex)
    {
      std::cout << "error: " << ex.what() << std::endl;
    }

  return EXIT_FAILURE;
}

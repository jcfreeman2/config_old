#include <stdlib.h>

#include <iostream>

#include <boost/program_options.hpp>

#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "config/Configuration.hpp"


int
main(int argc, char *argv[])
{
  std::string output_file, db_name, classes, objects, files, format("json");
  bool apply_fix(false);

  boost::program_options::options_description desc("Export config data using boost property tree.\n\nOptions/Arguments");

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
          "objects,i",
          boost::program_options::value<std::string>(&objects),
          "regex defining object IDs; ignore if empty"
        )
        (
          "files,f",
          boost::program_options::value<std::string>(&files),
          "regex defining data files; ignore if empty"
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
          "fix,x",
          "fix arrays output format:\n* enforce empty arrays for json;\n* remove unnamed xml tags"
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

      boost::program_options::notify(vm);

      if (vm.count("fix"))
        apply_fix = true;

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
      std::string fix_empty_arrays((apply_fix && format == "json") ? "<-- empty-p3-element -->" : "");

      db.export_data(pt, classes, objects, files, fix_empty_arrays);

      std::string in, out;

        {
          std::ostringstream buf;
          if  (format == "json")
            boost::property_tree::json_parser::write_json(buf, pt);
          else if (format == "xml")
            boost::property_tree::xml_parser::write_xml(buf, pt, boost::property_tree::xml_writer_make_settings<std::string>(' ', 4));
          else
            boost::property_tree::info_parser::write_info(buf, pt, boost::property_tree::info_writer_make_settings(' ', 4));

          in = buf.str();
        }

      if (!apply_fix || format == "info")
        out = std::move(in);
      else
        {
          out.reserve(in.size());
          std::string::size_type pos = 0, fix_pos;

          if (format == "json")
            {
              while ((fix_pos = in.find(fix_empty_arrays, pos)) != std::string::npos)
                {
                  std::string::size_type start = in.rfind('[', fix_pos);
                  std::string::size_type end = in.find(']', fix_pos);

                  if (start != std::string::npos && end != std::string::npos)
                    {
                      out.append(in, pos, start + 1 - pos);
                      pos = end;
                    }
                  else
                    break;
                }
            }
          else
            {
              // remove unnamed xml tags: replace "<>FOO</>" by "FOO"
              while ((fix_pos = in.find("<>", pos)) != std::string::npos)
                {
                  std::string::size_type next = in.find("</>", fix_pos);

                  if (next != std::string::npos)
                    {
                      out.append(in, pos, fix_pos - pos);
                      out.append(in, fix_pos+2, next - fix_pos - 2);
                      pos = next + 3;
                    }
                  else
                    break;
                }
            }

          out.append(in, pos);
        }

      if (!output_file.empty())
        {
          std::ofstream f(output_file);
          f.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
          f << out;
          f.close();
        }
      else
        std::cout << out;

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

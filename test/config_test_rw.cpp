//#include <stdlib.h>
#include <stdint.h>

#include <iostream>
#include <string>

#include <config/Configuration.h>
#include <config/ConfigObject.h>

ERS_DECLARE_ISSUE(
  config_test_rw,
  BadCommandLine,
  "bad command line: " << reason,
  ((const char*)reason)
)

ERS_DECLARE_ISSUE(
  config_test_rw,
  ConfigException,
  "caught daq::config::Exception exception",
)

static void
usage()
{
  std::cout << 
    "Usage: config_test_rw -d data_name -s schema_name -p plugin_spec\n"
    "\n"
    "Options/Arguments:\n"
    "       -d data_name      name of creating data file\n"
    "       -s schema_name    name of including schema file\n"
    "       -p plugin_spec    config plugin specification (oksconfig | rdbconfig:server-name)\n"
    "\n"
    "Description:\n"
    "       The utility tests creation of files and objects using different plugins.\n\n";
}

static void
no_param(const char * s)
{
  std::ostringstream text;
  text << "no parameter for " << s << " provided";
  ers::fatal(config_test_rw::BadCommandLine(ERS_HERE, text.str().c_str()));
  exit(EXIT_FAILURE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T> void set_value(ConfigObject& o, const std::string& name, T value)
{
  o.set_by_val(name, value);
}

template<class T> void set_ref(ConfigObject& o, const std::string& name, T value)
{
  o.set_by_ref(name, value);
}

template<class T> void check_value(ConfigObject& o, const std::string& name, T v1)
{
  T v2;
  o.get(name, v2);
  if(v1 != v2) {
    std::cerr << "ERROR reading attribute: \'" << name << '\'' << std::endl; 
  }
  else {
    std::cout << "TEST " << name << " of " << o << " is OK\n";
  }
}

void check_object(ConfigObject& o, const std::string& name, ConfigObject * o1)
{
  ConfigObject o2;
  o.get(name, o2);
  if(o1 == 0) {
    if(!o2.is_null()) {
      std::cerr << "ERROR reading relationship: \'" << name << "\' (read an object instead of NULL)" << std::endl; 
    }
  }
  else {
    if(o2.is_null()) {
      std::cerr << "ERROR reading relationship: \'" << name << "\' (read NULL instead of object)" << std::endl; 
    }
    else {
      if(!(o2 == *o1)) {
        std::cerr << "ERROR reading relationship: \'" << name << "\' (read and wrote objects are different)" << std::endl; 
      }
    }
  }

  std::cout << "TEST value of " << name << " relationship of object " << o << " is OK: read " << o2 << std::endl;
}

void check_objects(ConfigObject& o, const std::string& name, const std::vector<const ::ConfigObject*> o1)
{
  std::vector<ConfigObject> o2;

  o.get(name, o2);

  if(o1.size() != o2.size()) {
    std::cerr << "ERROR reading relationship: \'" << name << "\' (read vector of different size)" << std::endl; 
  }
  else {
    for(unsigned int i = 0; i < o1.size(); ++i) {
      if(!(*o1[i] == o2[i])) {
        std::cerr << "ERROR reading relationship: \'" << name << "\' (objects " << i << " are different)" << std::endl; 
      }
    }
  }

  std::cout << "TEST values of " << name << " relationship of object " << o << " is OK: read ";
  for(unsigned int i = 0; i < o1.size(); ++i) {
    if(i != 0) std::cout << ", ";
    std::cout << o2[i];
  } 
  std::cout << std::endl; 
}

void check_file_path(ConfigObject& o, const std::string& file_name)
{
  std::string value = o.contained_in();

  std::cout << "TEST object " << &o << " is contained in \'" << value << "\': ";

  if(value == file_name)
    {
      std::cout << "OK\n";
    }
  else
    {
      std::cout << "FAILED (expected \'" << file_name << "\')\n";
    }
}


void check_rename(ConfigObject& o, const std::string& name)
{
  std::cout << "TEST object " << &o << " was renamed to \'" << name << "\': ";

  if(name == o.UID())
    {
      std::cout << "OK\n";
    }
  else
    {
      std::cout << "FAILED\n";
    }
}


#define INIT(T, X, V)            \
for(T v = X - 16; v <= X;) {     \
  V.push_back(++v);              \
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



int main(int argc, char *argv[])
{
  const char * schema_name = 0;
  const char * data_name = 0;
  const char * plugin_name = 0;

  for(int i = 1; i < argc; i++) {
    const char * cp = argv[i];

    if(!strcmp(cp, "-h") || !strcmp(cp, "--help")) {
      usage();
      return 0;
    }
    else if(!strcmp(cp, "-d") || !strcmp(cp, "--data-name")) {
      if(++i == argc) { no_param(cp); } else { data_name = argv[i]; }
    }
    else if(!strcmp(cp, "-s") || !strcmp(cp, "--schema-name")) {
      if(++i == argc) { no_param(cp); } else { schema_name = argv[i]; }
    }
    else if(!strcmp(cp, "-p") || !strcmp(cp, "--plugin-spec")) {
      if(++i == argc) { no_param(cp); } else { plugin_name = argv[i]; }
    }
    else {
      std::ostringstream text;
      text << "unexpected parameter: \'" << cp << "\'; run command with --help to see valid command line options.";
      ers::fatal(config_test_rw::BadCommandLine(ERS_HERE, text.str().c_str()));
      return (EXIT_FAILURE);
    }
  }

  if(!data_name) {
    ers::fatal(config_test_rw::BadCommandLine(ERS_HERE, "no data filename given"));
    return (EXIT_FAILURE);
  }

  if(!schema_name) {
    ers::fatal(config_test_rw::BadCommandLine(ERS_HERE, "no schema filename given"));
    return (EXIT_FAILURE);
  }

  if(!plugin_name) {
    ers::fatal(config_test_rw::BadCommandLine(ERS_HERE, "no plugin specification given (oksconfig, rdbconfig:server-name)"));
    return (EXIT_FAILURE);
  }

  try {
    ::Configuration db(plugin_name);

    db.create(data_name, std::list<std::string>(1,schema_name));

    ConfigObject o1;
    db.create(data_name, "Dummy", "#1", o1);

    ConfigObject o2;
    db.create(data_name, "Dummy", "#2", o2);

    ConfigObject o3;
    db.create(data_name, "Second", "#3", o3);

    ConfigObject o4;
    db.create(data_name, "Third", "#4", o4);

    ConfigObject o5;
    db.create(data_name, "Third", "#5", o5);

    ConfigObject o6;
    db.create(data_name, "Third", "#6", o6);


       // put objects into implementation cache

    check_value(o1, "bool", false);
    check_value(o2, "bool", false);
    check_value(o3, "bool", false);
    check_value(o4, "bool", false);
    check_value(o5, "bool", false);
    check_value(o6, "bool", false);


    bool        bool_value   (true);
    int8_t      int8_value   (0x7F);
    uint8_t     uint8_value  (0xFF);
    int16_t     int16_value  (0x7FFF);
    uint16_t    uint16_value (0xFFFF);
    int32_t     int32_value  (0x7FFFFFFF);
    int32_t     uint32_value (0xFFFFFFFF);
    int64_t     int64_value  ((uint64_t)(-1)/2-1);
    uint64_t    uint64_value ((uint64_t)(-1));
    float       float_value  (123.456);
    double      double_value (1234567890.123456);
    std::string string_value ("This is a test string.");
    std::string enum_value   ("FIRST");
    std::string class_value  ("Third");

    std::vector<bool>        bool_values;    bool_values.push_back(true); bool_values.push_back(false); bool_values.push_back(true);
    std::vector<int8_t>      int8_values;    INIT(int8_t, 0x7E, int8_values);
    std::vector<uint8_t>     uint8_values;   INIT(uint8_t, 0xFE, uint8_values);
    std::vector<int16_t>     int16_values;   INIT(int16_t, 0x7FFE, int16_values);
    std::vector<uint16_t>    uint16_values;  INIT(uint16_t, 0xFFFE, uint16_values);
    std::vector<int32_t>     int32_values;   INIT(int32_t, 0x7FFFFFFE, int32_values);
    std::vector<int32_t>     uint32_values;  INIT(uint32_t, 0xFFFFFFFE, uint32_values);
    std::vector<int64_t>     int64_values;   INIT(int64_t, (std::numeric_limits<int64_t>::max()-2), int64_values);
    std::vector<uint64_t>    uint64_values;  INIT(uint64_t, (std::numeric_limits<uint64_t>::max()-1), uint64_values);
    std::vector<float>       float_values;   INIT(float, 123.456, float_values);
    std::vector<double>      double_values;  INIT(double, 1234567890.123456, double_values);
    std::vector<std::string> strings_values; strings_values.push_back("test20"); strings_values.push_back("test30"); strings_values.push_back("test10");
    std::vector<std::string> enum_values;    enum_values.push_back("THIRD"); enum_values.push_back("SECOND"); enum_values.push_back("FIRST");
    std::vector<std::string> class_values;   class_values.push_back("Dummy"); class_values.push_back("Second"); class_values.push_back("Third");

    set_ref(o1, "bool_vector", bool_values);
    set_ref(o1, "sint8_vector", int8_values);
    set_ref(o1, "uint8_vector", uint8_values);
    set_ref(o1, "sint16_vector", int16_values);
    set_ref(o1, "uint16_vector", uint16_values);
    set_ref(o1, "sint32_vector", int32_values);
    set_ref(o1, "uint32_vector", int32_values);
    set_ref(o1, "sint64_vector", int64_values);
    set_ref(o1, "uint64_vector", uint64_values);
    set_ref(o1, "float_vector", float_values);
    set_ref(o1, "double_vector", double_values);
    set_ref(o1, "string_vector", strings_values);
    o1.set_enum("enum_vector", enum_values);
    o1.set_class("classref_vector", class_values);

    set_value(o1, "bool",   bool_value);
    set_value(o1, "sint8",  int8_value);
    set_value(o1, "uint8",  uint8_value);
    set_value(o1, "sint16", int16_value);
    set_value(o1, "uint16", uint16_value);
    set_value(o1, "sint32", int32_value);
    set_value(o1, "uint32", uint32_value);
    set_value(o1, "sint64", int64_value);
    set_value(o1, "uint64", uint64_value);
    set_value(o1, "float",  float_value);
    set_value(o1, "double", double_value);
    set_ref(o1, "string", string_value);
    o1.set_enum("enum", enum_value);
    o1.set_class("classref", class_value);

    std::vector<const ::ConfigObject*> vec4; vec4.push_back(&o1); vec4.push_back(&o2);
    std::vector<const ::ConfigObject*> vec5; vec5.push_back(&o3); vec5.push_back(&o4);
    std::vector<const ::ConfigObject*> vec6; vec6.push_back(&o3);

    o3.set_objs("Dummy", vec4);
    o3.set_obj("Another", &o1);

    o4.set_objs("Dummy", vec4);
    o4.set_obj("Another", &o2);
    o4.set_obj("Single", &o6);

    o5.set_objs("Dummy", vec4);
    o5.set_obj("Another", &o3);
    o5.set_obj("Single", 0);
    o5.set_objs("Seconds", vec5);

    o6.set_objs("Dummy", vec4);
    o6.set_obj("Another", &o3);
    o6.set_obj("Single", 0);
    o6.set_objs("Seconds", vec6);

    check_value(o1, "bool",   bool_value);
    check_value(o1, "sint8",  int8_value);
    check_value(o1, "uint8",  uint8_value);
    check_value(o1, "sint16", int16_value);
    check_value(o1, "uint16", uint16_value);
    check_value(o1, "sint32", int32_value);
    check_value(o1, "uint32", uint32_value);
    check_value(o1, "sint64", int64_value);
    check_value(o1, "uint64", uint64_value);
    check_value(o1, "float",  float_value);
    check_value(o1, "double", double_value);
    check_value(o1, "string", string_value);
    check_value(o1, "enum", enum_value);
    check_value(o1, "classref", class_value);
    check_value(o1, "bool_vector", bool_values);
    check_value(o1, "sint8_vector", int8_values);
    check_value(o1, "uint8_vector", uint8_values);
    check_value(o1, "sint16_vector", int16_values);
    check_value(o1, "uint16_vector", uint16_values);
    check_value(o1, "sint32_vector", int32_values);
    check_value(o1, "uint32_vector", int32_values);
    check_value(o1, "sint64_vector", int64_values);
    check_value(o1, "uint64_vector", uint64_values);
    check_value(o1, "float_vector", float_values);
    check_value(o1, "double_vector", double_values);
    check_value(o1, "string_vector", strings_values);
    check_value(o1, "enum_vector", enum_values);
    check_value(o1, "classref_vector", class_values);

    check_objects(o3, "Dummy", vec4);
    check_object(o3, "Another", &o1);
    check_objects(o4, "Dummy", vec4);
    check_object(o4, "Another", &o2);
    check_object(o4, "Single", &o6);
    check_objects(o5, "Seconds", vec5);
    check_objects(o6, "Dummy", vec4);
    check_object(o6, "Another", &o3);
    check_object(o6, "Single", 0);
    check_objects(o6, "Seconds", vec6);

    check_file_path(o1, data_name);
    check_file_path(o3, data_name);
    check_file_path(o4, data_name);

    {
      std::list<std::string> modified;

      db.get_updated_dbs(modified);

      std::cout << "There are updated " << modified.size() << " files:\n";
      for(std::list<std::string>::const_iterator i = modified.begin(); i != modified.end(); ++i) {
        std::cout << " * \"" << *i << "\"" << std::endl;
      }
    }

    db.commit("test application (config/test/config_test_rw.cpp): create first data");


      // create file names for 2 intermediate and 4 leave files;
      // file $F1 will include $F11 and $F12
      // file $F2 will include $F21, $F22 and $F12
      // existing file $data_name will include $F1 and $F2

    std::string f11(data_name); f11 += ".1.1"; 
    std::string f12(data_name); f12 += ".1.2"; 
    std::string f21(data_name); f21 += ".2.1"; 
    std::string f22(data_name); f22 += ".2.2"; 

    db.create(f11, std::list<std::string>(1,schema_name));
    db.create(f12, std::list<std::string>(1,schema_name));
    db.create(f21, std::list<std::string>(1,schema_name));
    db.create(f22, std::list<std::string>(1,schema_name));

    std::string f1(data_name); f1 += ".1"; 
    std::string f2(data_name); f2 += ".2"; 

    {
      std::list<std::string> includes(1,schema_name); includes.push_back(f11); includes.push_back(f12);
      db.create(f1, includes);
    }

    {
      std::list<std::string> includes(1,schema_name); includes.push_back(f21); includes.push_back(f22); includes.push_back(f12);
      db.create(f2, includes);
    }

    struct {
      const std::string * file;
      const char * id;
    } data [12] = {
      {&f1, "f1-1"},
      {&f1, "f1-2"},
      {&f2, "f2-1"},
      {&f2, "f2-2"},
      {&f11, "f11-1"},
      {&f11, "f11-2"},
      {&f12, "f12-1"},
      {&f12, "f12-2"},
      {&f21, "f21-1"},
      {&f21, "f21-2"},
      {&f22, "f22-1"},
      {&f22, "f22-2"}
    };

    for(int i = 0; i < 12; ++i) {
      ConfigObject o;
      db.create(*data[i].file, "Dummy", data[i].id, o);
      check_file_path(o, *data[i].file);
    }

    db.add_include(data_name, f1);
    db.add_include(data_name, f2);

    db.commit("test application (config/test/config_test_rw.cpp): create 6 nested files");

    std::cout << "\n\nTEST VALIDITY OF OBJECTS AFTER REMOVAL OF INCLUDES: Removing include \"" << f1 << "\"\n\n";

    db.remove_include(data_name, f1);

    const char * removed_objects_by_include[] = {"f1-1", "f1-2", "f11-1", "f11-2"}; // these objects have to be removed (note, f12 is still included by f2)

    for(int i = 0; i < 12; ++i) {
      std::cout << "TEST object " << data[i].id << " existence after removal of includes: ";
      bool state(false);
      try {
        ConfigObject o;
        db.get("Dummy", data[i].id, o);
	for(int j = 0; j < 4; ++j) {
	  if(!strcmp(removed_objects_by_include[j], data[i].id)) {
	    std::cout << "FAILED, object was not removed"; state = true; break;
	  }
	}
	if(state == false) {
	  std::cout << "OK, object was not removed";
	}
      }
      catch(daq::config::NotFound& ex) {
	for(int j = 0; j < 4; ++j) {
	  if(!strcmp(removed_objects_by_include[j], data[i].id)) {
	    std::cout << "OK, object was removed"; state = true; break;
	  }
	}
	if(state == false) {
	  std::cout << "FAILED, object was removed";
	}
      }
      std::cout << std::endl;
    }

    std::cout << "\n\nTEST VALIDITY OF OBJECTS AFTER REMOVAL OF COMPOSITE PARENT: Destroying object \"" << o5 << "\"\n\n";

    const char * existing_objects[] = {"#1", "#2", "#3", "#4", "#5", "#6" };
    const char * removed_objects_by_composite_parent[] = {"#4", "#5"};  // these objects have to be removed (note, #3 is still referenced by #6

    db.destroy_obj(o5);

    std::cout << "TEST deleted object " << o5.UID() << " existence: " << (o5.is_deleted() ? "OK (is_deleted returns TRUE)" : "FAILED (is_deleted returns FALSE)") << std::endl;

    check_file_path(o1, data_name);
    check_file_path(o3, data_name);
    check_file_path(o6, data_name);

    for(int i = 0; i < 6; ++i) {
      std::cout << "TEST object " << existing_objects[i] << " existence: ";

      bool state(false);
      try {
        ConfigObject o;
        db.get("Dummy", existing_objects[i], o);
	for(int j = 0; j < 2; ++j) {
	  if(!strcmp(removed_objects_by_composite_parent[j], existing_objects[i])) {
	    std::cout << "FAILED, object was not removed"; state = true; break;
	  }
	}
	if(state == false) {
	  std::cout << "OK, object was not removed";
	}
      }
      catch(daq::config::NotFound& ex) {
	for(int j = 0; j < 2; ++j) {
	  if(!strcmp(removed_objects_by_composite_parent[j], existing_objects[i])) {
	    std::cout << "OK, object was removed"; state = true; break;
	  }
	}
	if(state == false) {
	  std::cout << "FAILED, object was removed";
	}
      }
      std::cout << std::endl;
    }


    std::cout << "\n\nTEST INCLUDES\n\n";

    std::list<std::string> includes;
    db.get_includes(data_name, includes);

    std::cout << "* file \"" << data_name << "\" includes " << includes.size() << " files:\n";
    for (auto& x : includes)
      std::cout << " - " << x << std::endl;

    std::cout << "test " << ((includes.size() == 2) ? "PASSED" : "FAILED") << std::endl;

    includes.clear();
    db.get_includes("", includes);

    std::cout << "* there is " << includes.size() << " top-level files:\n";
    for (auto& x : includes)
      std::cout << " - " << x << std::endl;

    std::cout << "test " << ((includes.size() == 1) ? "PASSED" : "FAILED") << std::endl;


    std::cout << "\n\nTEST RENAME\n\n";

    o1.rename("#new1");
    check_rename(o1, "#new1");

    // rename existing object to deleted

    std::cout << "TEST deleted object " << o4.UID() << " existence: " << (o4.is_deleted() ? "OK" : "FAILED") << std::endl;
    const std::string deleted_name(o4.UID());
    o6.rename(deleted_name);
    check_rename(o6, deleted_name);
    std::cout << "TEST deleted object " << deleted_name << " after renamed existing object to it's ID: " << (!o4.is_deleted() ? "OK" : "FAILED") << std::endl;
    check_rename(o4, deleted_name);

    return 0;
  }
  catch (daq::config::Exception & ex) {
    ers::fatal(config_test_rw::ConfigException(ERS_HERE, ex));
  }

  return (EXIT_FAILURE);
}

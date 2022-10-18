/**
 * @file src/python/config.cpp
 * @author <a href="mailto:Andre.dos.Anjos@cern.ch">Andre ANJOS</a> 
 *
 * @brief Boost.Python interface to the "config" namespace
 */

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <memory>
#include <string>
#include <list>
#include <vector>
#include <stdint.h>
#include <cstdio>

#include "config/ConfigObject.h"
#include "config/ConfigurationPointer.h"
#include "config/Schema.h"
#include "config/Errors.h"

using namespace boost::python;

static std::string make_ers_message(ers::Issue const& e) 
{
  const ers::Context& curr = e.context();
  std::ostringstream oss;
  oss << "[" << curr.package_name() << "]" 
      << curr.function_name() << "@" << curr.file_name() << "+"
      << curr.line_number() << ": " << e.message();
  if (e.cause()) oss << " caused by " << make_ers_message(*e.cause());
  return oss.str();
}

static void translate_ers_issue(ers::Issue const& e)
{
  PyErr_SetString(PyExc_RuntimeError, make_ers_message(e).c_str());
}

template <typename T> static T get_value(ConfigObject& co, 
                                  const std::string& attrname) {
  T retval;
  co.get(attrname, retval);
  return retval;
}

template <typename T> static T* get_new(ConfigObject& co,
                                 const std::string& attrname) {
  T* retval = new T;
  co.get(attrname, *retval);
  return retval;
}

static ConfigObject* co_get_obj(ConfigObject& co, const std::string& attrname) {
  ConfigObject* obj = get_new<ConfigObject>(co, attrname);
  if (obj->is_null()) {
    delete obj; 
    return 0;
  }
  return obj;
}

template <typename T> static
  boost::python::list get_list(ConfigObject& co, const std::string& attrname) {
  boost::python::list l;
  std::vector<T> data;
  co.get(attrname, data);
  for (unsigned i=0; i<data.size(); ++i) l.append(data[i]);
  return l;
}

static boost::python::list get_references(ConfigObject& co,
    const std::string& relationship_name="*", unsigned long rlevel=0) {
  boost::python::list l;
  std::vector<ConfigObject> data;
  co.referenced_by(data, relationship_name, false, rlevel, 0);
  for (unsigned i=0; i<data.size(); ++i) l.append(data[i]);
  return l;
}

static void set_string_list_generic
  (ConfigObject& co, const std::string& attrname, boost::python::list& l,
   void (ConfigObject::*f)(const std::string&, const std::vector<std::string>&)) {
  std::vector<std::string> data;
  data.reserve(PyList_Size(l.ptr()));
  for (long i=0; i<PyList_Size(l.ptr()); ++i) {
    char const* tmp = extract<char const*>(PyList_GetItem(l.ptr(), i));
    data.push_back(tmp);
  }
  (co.*f)(attrname, data);
}
static void set_string_list
  (ConfigObject& co, const std::string& attrname, boost::python::list& l) {
  set_string_list_generic(co, attrname, l, 
      &ConfigObject::set_by_ref<const std::vector<std::string> >);
}
static void set_enum_list
  (ConfigObject& co, const std::string& attrname, boost::python::list& l) {
  set_string_list_generic(co, attrname, l, &ConfigObject::set_enum);
}
static void set_class_list
  (ConfigObject& co, const std::string& attrname, boost::python::list& l) {
  set_string_list_generic(co, attrname, l, &ConfigObject::set_class);
}
static void set_date_list
  (ConfigObject& co, const std::string& attrname, boost::python::list& l) {
  set_string_list_generic(co, attrname, l, &ConfigObject::set_date);
}
static void set_time_list
  (ConfigObject& co, const std::string& attrname, boost::python::list& l) {
  set_string_list_generic(co, attrname, l, &ConfigObject::set_time);
}

static void set_co_list
  (ConfigObject& co, const std::string& attrname, boost::python::list& l) {
  std::vector<const ConfigObject*> data;
  data.reserve(PyList_Size(l.ptr()));
  for (long i=0; i<PyList_Size(l.ptr()); ++i) {
    const ConfigObject& tmp = 
      extract<ConfigObject&>(PyList_GetItem(l.ptr(), i));
    data.push_back(&tmp);
  }
  co.set_objs(attrname, data);
}

template <typename T> static void set_list 
(ConfigObject& co, const std::string& attrname, boost::python::list& l) {
  std::vector<T> data;
  data.reserve(PyList_Size(l.ptr()));
  for (long i=0; i<PyList_Size(l.ptr()); ++i) data.push_back(extract<T>(l[i]));
  co.set_by_ref(attrname, data);
}

static ConfigObject* create_obj_str(python::ConfigurationPointer& conf, const std::string& at,
    const std::string& class_name, const std::string& id) 
{
  ConfigObject* retval = new ConfigObject;
  conf->create(at, class_name, id, *retval);
  return retval;
}

static ConfigObject* create_obj_co(python::ConfigurationPointer& conf, const ConfigObject& at,
    const std::string& class_name, const std::string& id) 
{
  ConfigObject* retval = new ConfigObject;
  conf->create(at, class_name, id, *retval);
  return retval;
}

static ConfigObject* config_get_obj(python::ConfigurationPointer& conf,
    const std::string& class_name, const std::string& id) 
{
  ConfigObject* retval = new ConfigObject;
  conf->get(class_name, id, *retval);
  if (retval->is_null()) {
    delete retval;
    retval = 0;
  }
  return retval;
}

static boost::python::list get_objs(python::ConfigurationPointer& conf,
    const std::string& class_name, const std::string& query="")
{
  std::vector<ConfigObject> objs;
  conf->get(class_name, objs, query);
  boost::python::list retval;
  for (unsigned i=0; i < objs.size(); ++i) 
    if(!objs[i].is_null()) retval.append(objs[i]);
  return retval;
}

static boost::python::dict
attributes(python::ConfigurationPointer& conf, const std::string& class_name, bool all)
{
  boost::python::dict retval;

  const daq::config::class_t& c = conf->get_class_info(class_name, !all);

  for (const auto& x : c.p_attributes)
    {
      boost::python::dict properties;

      properties["type"] = daq::config::attribute_t::type(x.p_type);
      if(x.p_range.empty())
        properties["range"] = properties.get("dummy"); // set to None
      else
        properties["range"] = x.p_range;
      properties["description"] = x.p_description;
      properties["multivalue"] = x.p_is_multi_value;
      properties["not-null"] = x.p_is_not_null;
      if(x.p_default_value.empty())
        properties["init-value"] = properties.get("dummy"); // set to None
      else
        properties["init-value"] = x.p_default_value;

      retval[x.p_name] = properties;
    }

  return retval;
}

static boost::python::dict
relations(python::ConfigurationPointer& conf, const std::string& class_name, bool all)
{
  boost::python::dict retval;

  const daq::config::class_t& c = conf->get_class_info(class_name, !all);

  for (const auto& x : c.p_relationships)
    {
      boost::python::dict properties;

      properties["type"] = x.p_type;
      properties["description"] = x.p_description;
      properties["multivalue"] = (x.p_cardinality == daq::config::zero_or_many || x.p_cardinality == daq::config::one_or_many);
      properties["aggregation"] = x.p_is_aggregation;
      properties["not-null"] = (x.p_cardinality == daq::config::only_one || x.p_cardinality == daq::config::one_or_many);

      retval[x.p_name] = properties;
    }

  return retval;
}

static boost::python::list
superclasses(python::ConfigurationPointer& conf, const std::string& class_name, bool all)
{
  boost::python::list retval;

  const daq::config::class_t& c = conf->get_class_info(class_name, !all);

  for (const auto& x : c.p_superclasses)
    retval.append(x);

  return retval;
}

static boost::python::list
subclasses(python::ConfigurationPointer& conf, const std::string& class_name, bool all)
{
  boost::python::list retval;

  const daq::config::class_t& c = conf->get_class_info(class_name, !all);

  for (const auto& x : c.p_subclasses)
    retval.append(x);

  return retval;
}

static boost::python::list
classes(python::ConfigurationPointer& conf)
{
  boost::python::list retval;

  for (const auto& it : conf->superclasses())
    retval.append(*it.first);

  return retval;
}

static void create_db(python::ConfigurationPointer& conf, 
                      const std::string& db_name, 
                      boost::python::list& includes)
{
  std::list<std::string> l;
  for (long i=0; i<PyList_Size(includes.ptr()); ++i) {
    char const* tmp = extract<char const*>(PyList_GetItem(includes.ptr(), i));
    l.push_back(tmp);
  }
  conf->create(db_name, l);
}

static boost::python::list get_includes(python::ConfigurationPointer& conf,
                                        const std::string& db_name)
{
  std::list<std::string> l;
  conf->get_includes(db_name, l);
  boost::python::list retval;
  for (std::list<std::string>::const_iterator i=l.begin(); i!=l.end(); ++i) 
    retval.append(*i);
  return retval;
}

static bool test_object(python::ConfigurationPointer& conf, 
    const std::string& class_name, 
    const std::string& id, unsigned long rlevel=0) {
  return conf->test_object(class_name, id, rlevel);
}

static void destroy_obj(python::ConfigurationPointer& conf, ConfigObject& obj) 
{
  conf->destroy_obj(obj);
}

static bool loaded(python::ConfigurationPointer& conf) 
{
  return conf->loaded();
}

static void load(python::ConfigurationPointer& conf, const std::string& s)
{
  conf->load(s);
}

static void unload(python::ConfigurationPointer& conf)
{
  conf->unload();
}

static void add_include(python::ConfigurationPointer& conf,
    const std::string& db, const std::string& inc)
{
  conf->add_include(db, inc);
}

static void remove_include(python::ConfigurationPointer& conf,
    const std::string& db, const std::string& inc)
{
  conf->remove_include(db, inc);
}

static void commit(python::ConfigurationPointer& conf, 
		   const std::string& comment = "python plug-in")
{
  conf->commit(comment);
}

static void db_abort(python::ConfigurationPointer& conf)
{
  conf->abort();
}

str get_impl_spec(python::ConfigurationPointer& conf) {
  return str(conf->get_impl_spec());
}

str get_impl_name(python::ConfigurationPointer& conf) {
  return str(conf->get_impl_name());
}

str get_impl_param(python::ConfigurationPointer& conf) {
  return str(conf->get_impl_param());
}

BOOST_PYTHON_FUNCTION_OVERLOADS(get_references_overloads, get_references, 1, 3)
BOOST_PYTHON_FUNCTION_OVERLOADS(test_object_overloads, test_object, 3, 4)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(set_obj_overloads, set_obj, 2, 3)
BOOST_PYTHON_FUNCTION_OVERLOADS(commit_overload, commit, 1, 2)

BOOST_PYTHON_MODULE(libpyconfig)
{
  register_exception_translator<daq::config::Exception>(&translate_ers_issue);

  class_<ConfigObject>("ConfigObject", init<>())
    .def(init<const ConfigObject&>())
    .def("get_string", &get_value<std::string>)
    .def("get_bool", &get_value<bool>)
    .def("get_s8", &get_value<int8_t>)
    .def("get_u8", &get_value<uint8_t>)
    .def("get_s16", &get_value<int16_t>)
    .def("get_u16", &get_value<uint16_t>)
    .def("get_s32", &get_value<int32_t>)
    .def("get_u32", &get_value<uint32_t>)
    .def("get_s64", &get_value<int64_t>)
    .def("get_u64", &get_value<uint64_t>)
    .def("get_float", &get_value<float>)
    .def("get_double", &get_value<double>)
    .def("get_obj", &co_get_obj, return_value_policy<manage_new_object>())
    .def("get_string_vec", &get_list<std::string>)
    .def("get_bool_vec", &get_list<bool>)
    .def("get_s8_vec", &get_list<int8_t>)
    .def("get_u8_vec", &get_list<uint8_t>)
    .def("get_s16_vec", &get_list<int16_t>)
    .def("get_u16_vec", &get_list<uint16_t>)
    .def("get_s32_vec", &get_list<int32_t>)
    .def("get_u32_vec", &get_list<uint32_t>)
    .def("get_s64_vec", &get_list<int64_t>)
    .def("get_u64_vec", &get_list<uint64_t>)
    .def("get_float_vec", &get_list<float>)
    .def("get_double_vec", &get_list<double>)
    .def("get_objs", &get_list<ConfigObject>)
    .def("referenced_by", (boost::python::list(*)(ConfigObject&,
      const std::string&, unsigned long))0, get_references_overloads()) 
    .def("set_string", &ConfigObject::set_by_val<std::string>)
    .def("set_enum", (void (ConfigObject::*)(const std::string&, const std::string&))&ConfigObject::set_enum)
    .def("set_class", (void (ConfigObject::*)(const std::string&, const std::string&))&ConfigObject::set_class)
    .def("set_date", (void (ConfigObject::*)(const std::string&, const std::string&))&ConfigObject::set_date)
    .def("set_time", (void (ConfigObject::*)(const std::string&, const std::string&))&ConfigObject::set_time)
    .def("set_bool", &ConfigObject::set_by_val<bool>)
    .def("set_s8", &ConfigObject::set_by_val<int8_t>)
    .def("set_u8", &ConfigObject::set_by_val<uint8_t>)
    .def("set_s16", &ConfigObject::set_by_val<int16_t>)
    .def("set_u16", &ConfigObject::set_by_val<uint16_t>)
    .def("set_s32", &ConfigObject::set_by_val<int32_t>)
    .def("set_u32", &ConfigObject::set_by_val<uint32_t>)
    .def("set_s64", &ConfigObject::set_by_val<int64_t>)
    .def("set_u64", &ConfigObject::set_by_val<uint64_t>)
    .def("set_float", &ConfigObject::set_by_val<float>)
    .def("set_double", &ConfigObject::set_by_val<double>)
    .def("set_obj", &ConfigObject::set_obj, set_obj_overloads()[with_custodian_and_ward<1,3>()])
    .def("set_string_vec", &set_string_list)
    .def("set_enum_vec", &set_enum_list)
    .def("set_class_vec", &set_class_list)
    .def("set_date_vec", &set_date_list)
    .def("set_time_vec", &set_time_list)
    .def("set_bool_vec", &set_list<bool>)
    .def("set_s8_vec", &set_list<int8_t>)
    .def("set_u8_vec", &set_list<uint8_t>)
    .def("set_s16_vec", &set_list<int16_t>)
    .def("set_u16_vec", &set_list<uint16_t>)
    .def("set_s32_vec", &set_list<int32_t>)
    .def("set_u32_vec", &set_list<uint32_t>)
    .def("set_s64_vec", &set_list<int64_t>)
    .def("set_u64_vec", &set_list<uint64_t>)
    .def("set_float_vec", &set_list<float>)
    .def("set_double_vec", &set_list<double>)
    .def("set_objs", &set_co_list, with_custodian_and_ward<1,3>())
    .def("class_name", &ConfigObject::class_name, return_value_policy<copy_const_reference>())
    .def("UID", &ConfigObject::UID, return_value_policy<copy_const_reference>())
    .def("full_name", &ConfigObject::full_name)
    .def("contained_in", &ConfigObject::contained_in)
    .def("rename", &ConfigObject::rename)
    ;

  class_<python::ConfigurationPointer>("Configuration")
    .def(init<const std::string&>())
    .def(init<const python::ConfigurationPointer&>())
    .def("create_obj", &create_obj_str, return_value_policy<manage_new_object>())
    .def("create_obj", &create_obj_co, return_value_policy<manage_new_object>())
    .def("get_obj", &config_get_obj, return_value_policy<manage_new_object>())
    .def("get_objs", &get_objs)
    .def("attributes", &attributes)
    .def("relations", &relations)
    .def("superclasses", &superclasses)
    .def("subclasses", &subclasses)
    .def("classes", &classes)
    .def("destroy_obj", &destroy_obj)
    .def("loaded", &loaded)
    .def("unload", &unload)
    .def("load", &load)
    .def("create_db", &create_db) 
    .def("add_include", &add_include)
    .def("remove_include", &remove_include)
    .def("get_includes", &get_includes) 
    .def("commit", &commit, commit_overload())
    .def("abort", &db_abort)
    .def("test_object", (bool(*)(python::ConfigurationPointer&, const std::string&,
      const std::string&, unsigned long))0, test_object_overloads()) 
    .def("get_impl_spec", &get_impl_spec)
    .def("get_impl_name", &get_impl_name)
    .def("get_impl_param", &get_impl_param)
    ;
}

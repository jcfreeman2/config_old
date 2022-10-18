#include <stdlib.h>
#include <iostream>
#include <regex>
#include <sstream>

#include <dlfcn.h>

#include <ers/ers.h>
#include <ers/internal/SingletonCreator.h>

#include "config/Change.h"
#include "config/DalObject.h"
#include "config/DalObjectPrint.h"
#include "config/DalFactory.h"
#include "config/ConfigObject.h"
#include "config/ConfigAction.h"
#include "config/Configuration.h"
#include "config/ConfigurationImpl.h"
#include "config/Schema.h"

namespace daq {

  ERS_DEFINE_ISSUE_CXX( config, Exception, , )

  ERS_DEFINE_ISSUE_BASE_CXX(
    config,
    Generic,
    config::Exception,
    what,
    ,
    ((const char*)what)
  )

  ERS_DEFINE_ISSUE_BASE_CXX(
    config,
    NotFound,
    config::Exception,
    type << " \"" << data << "\" is not found",
    ,
    ((const char*)type)
    ((const char*)data)
  )

  ERS_DEFINE_ISSUE_BASE_CXX(
    config,
    DeletedObject,
    config::Exception,
    "object \'" << object_id << '@' << class_name << "\' was deleted",
    ,
    ((const char*)class_name)
    ((const char*)object_id)
  )

}

////////////////////////////////////////////////////////////////////////////////

void
Configuration::add_action(ConfigAction * ac)
{
  std::lock_guard<std::mutex> scoped_lock(m_actn_mutex);
  m_actions.push_back(ac);
}

void
Configuration::remove_action(ConfigAction * ac)
{
  std::lock_guard<std::mutex> scoped_lock(m_actn_mutex);
  m_actions.remove(ac);
}

void
Configuration::action_on_update(const ConfigObject& obj, const std::string& name)
{
  std::lock_guard<std::mutex> scoped_lock(m_actn_mutex);
  for (auto &i : m_actions)
    i->update(obj, name);
}

////////////////////////////////////////////////////////////////////////////////

static bool
check_prefetch_needs()
{
  return (getenv("TDAQ_DB_PREFETCH_ALL_DATA") != nullptr);
}

////////////////////////////////////////////////////////////////////////////////


Configuration::Configuration(const std::string& spec) :
    p_number_of_cache_hits(0), p_number_of_template_object_created(0), p_number_of_template_object_read(0), m_impl(nullptr), m_shlib_h(nullptr)
{
  std::string s;

  if (spec.empty())
    {
      if (const char *env = getenv("TDAQ_DB"))
        m_impl_spec = env;
    }
  else
    {
      m_impl_spec = spec;
    }

  if (m_impl_spec.empty())
    throw daq::config::Generic(ERS_HERE, "no database parameter found (check parameter of the constructor or value of TDAQ_DB environment variable)");

  std::string::size_type idx = m_impl_spec.find_first_of(':');

  if (idx == std::string::npos)
    {
      m_impl_name = m_impl_spec;
    }
  else
    {
      m_impl_name = m_impl_spec.substr(0, idx);
      m_impl_param = m_impl_spec.substr(idx + 1);
    }

  std::string plugin_name = std::string("lib") + m_impl_name + ".so";
  std::string impl_creator = std::string("_") + m_impl_name + "_creator_";

  // load plug-in

  m_shlib_h = dlopen(plugin_name.c_str(), RTLD_LAZY | RTLD_GLOBAL);

  if (!m_shlib_h)
    {
      std::ostringstream text;
      text << "failed to load implementation plug-in \'" << plugin_name << "\': \"" << dlerror() << '\"';
      throw(daq::config::Generic( ERS_HERE, text.str().c_str() ) );
    }

  // search in plug-in implementation creator function

  ConfigurationImpl *
  (*f)(const std::string& spec);

  f = (ConfigurationImpl*
  (*)(const std::string&))dlsym(m_shlib_h, impl_creator.c_str());

  char * error = 0;

  if ((error = dlerror()) != 0)
    {
      std::ostringstream text;
      text << "failed to find implementation creator function \'" << impl_creator << "\' in plug-in \'" << plugin_name << "\': \"" << error << '\"';
      throw(daq::config::Generic( ERS_HERE, text.str().c_str() ) );
    }


    // create implementation

  m_impl = (*f)(m_impl_param);

  if (m_impl)
    {
      m_impl->get_superclasses(p_superclasses);
      set_subclasses();
      m_impl->set(this);
    }

  if (check_prefetch_needs())
    m_impl->prefetch_all_data();

  ERS_DEBUG(2, "\n*** DUMP CONFIGURATION ***\n" << *this);
}


void
Configuration::print_profiling_info() noexcept
{
  std::lock_guard < std::mutex > scoped_lock(m_impl_mutex);

  std::cout << "Configuration profiler report:\n"
      "  number of created template objects: " << p_number_of_template_object_created << "\n"
      "  number of read template objects: " << p_number_of_template_object_read << "\n"
      "  number of cache hits: " << p_number_of_cache_hits << std::endl;

  const char * s = ::getenv("TDAQ_DUMP_CONFIG_PROFILER_INFO");
  if (s && !strcmp(s, "DEBUG"))
    {
      std::cout << "  Details of accessed objects:\n";

      for (auto & i : m_cache_map)
        {
          Cache<DalObject> *c = static_cast<Cache<DalObject>*>(i.second);
          std::cout << "    *** " << c->m_cache.size() << " objects is class \'" << *i.first << "\' were accessed ***\n";
          for (auto & j : c->m_cache)
            std::cout << "     - object \'" << j.first << '\'' << std::endl;
        }
    }

  if (m_impl)
    {
      m_impl->print_cache_info();
      m_impl->print_profiling_info();
    }
}

Configuration::~Configuration() noexcept
{
  if (::getenv("TDAQ_DUMP_CONFIG_PROFILER_INFO"))
    print_profiling_info();

  try
    {
      unload();

      if (m_shlib_h)
        {
          std::lock_guard<std::mutex> scoped_lock(m_impl_mutex);

          delete m_impl;
          m_impl = 0;
          //dlclose(m_shlib_h);
          m_shlib_h = 0;
        }
    }
  catch (daq::config::Generic& ex)
    {
      ers::error(ex);
    }
}

void
Configuration::get(const std::string& class_name, const std::string& id, ConfigObject& object, unsigned long rlevel, const std::vector<std::string> * rclasses)
{
  std::lock_guard<std::mutex> scoped_lock(m_impl_mutex);
  _get(class_name, id, object, rlevel, rclasses);
}

void
Configuration::_get(const std::string& class_name, const std::string& name, ConfigObject& object, unsigned long rlevel, const std::vector<std::string> * rclasses)
{
  try
    {
      m_impl->get(class_name, name, object, rlevel, rclasses);
    }
  catch (daq::config::Generic& ex)
    {
      std::ostringstream text;
      text << "failed to get object \'" << name << '@' << class_name << '\'';
      throw daq::config::Generic( ERS_HERE, text.str().c_str(), ex );
    }
}

void
Configuration::get(const std::string& class_name, std::vector<ConfigObject>& objects, const std::string& query, unsigned long rlevel, const std::vector<std::string> * rclasses)
{
  try
    {
      std::lock_guard<std::mutex> scoped_lock(m_impl_mutex);
      m_impl->get(class_name, objects, query, rlevel, rclasses);
    }
  catch (daq::config::Generic& ex)
    {
      std::ostringstream text;
      text << "failed to get objects of class \'" << class_name << '\'';
      if (!query.empty())
        {
          text << " with query \'" << query << '\'';
        }
      throw daq::config::Generic( ERS_HERE, text.str().c_str(), ex );
    }
}

void
Configuration::get(const ConfigObject& obj_from, const std::string& query, std::vector<ConfigObject>& objects, unsigned long rlevel, const std::vector<std::string> * rclasses)
{
  try
    {
      std::lock_guard<std::mutex> scoped_lock(m_impl_mutex);
      m_impl->get(obj_from, query, objects, rlevel, rclasses);
    }
  catch (daq::config::Generic& ex)
    {
      std::ostringstream text;
      text << "failed to get path \'" << query << "\' from object \'" << obj_from << '\'';
      throw daq::config::Generic( ERS_HERE, text.str().c_str(), ex );
    }
}

bool
Configuration::loaded() const noexcept
{
  return (m_impl != nullptr) ? m_impl->loaded() : false;
}

void
Configuration::load(const std::string& db_name)
{
  std::string name;

  if (db_name.empty())
    {
      if (!m_impl_spec.empty() && !m_impl_param.empty())
        {
          name = m_impl_param;
        }
      else
        {
          const char * s = ::getenv("TDAQ_DB_NAME");
          if (s == 0 || *s == 0)
            s = ::getenv("TDAQ_DB_DATA");

          if (s && *s)
            {
              name = s;
            }
          else
            {
              throw(daq::config::Generic( ERS_HERE, "no database name was provided" ) );
            }
        }
    }
  else
    {
      name = db_name;
    }

  std::lock_guard<std::mutex> scoped_lock(m_impl_mutex);

  // call config actions if any
    {
      std::lock_guard<std::mutex> scoped_lock(m_actn_mutex);
      for (auto & i : m_actions)
        {
          i->load();
        }
    }

  if (m_impl)
    {
      m_impl->open_db(name);
      m_impl->get_superclasses(p_superclasses);
      set_subclasses();
      m_impl->set(this);

      if(check_prefetch_needs())
        {
          m_impl->prefetch_all_data();
        }

      ERS_DEBUG(2, "\n*** DUMP CONFIGURATION ***\n" << *this);
    }
  else
    {
      throw daq::config::Generic( ERS_HERE, "no implementation loaded" );
    }
}

void
Configuration::unload()
{
  if (m_impl == nullptr)
    throw daq::config::Generic( ERS_HERE, "nothing to unload" );

  std::lock_guard<std::mutex> scoped_lock1(m_tmpl_mutex);  // always lock template objects mutex first
  std::lock_guard<std::mutex> scoped_lock2(m_impl_mutex);

  // call config actions if any
    {
      std::lock_guard<std::mutex> scoped_lock(m_actn_mutex);
      for(auto & i : m_actions)
        {
          i->unload();
        }
    }

  for(auto & i : m_cache_map)
    {
      delete i.second;
    }

  m_cache_map.clear();

    {
      std::lock_guard<std::mutex> scoped_lock3(m_else_mutex);

      for(auto& cb : m_callbacks)
        delete cb;

      for(auto& cb : m_pre_callbacks)
        delete cb;

      m_callbacks.clear();
      m_pre_callbacks.clear();

      m_impl->unsubscribe();

      for(auto& l : m_convert_map)
        {
          for(auto& a : *l.second)
            delete a;

          delete l.second;
        }

      m_convert_map.clear();
    }

  p_superclasses.clear();

  for(auto& j : p_direct_classes_desc_cache)
    delete j.second;

  for(auto& j : p_all_classes_desc_cache)
    delete j.second;

  p_direct_classes_desc_cache.clear();
  p_all_classes_desc_cache.clear();

  m_impl->close_db();
}

void
Configuration::create(const std::string& db_name, const std::list<std::string>& includes)
{
  if (m_impl == nullptr)
    throw daq::config::Generic( ERS_HERE, "no implementation loaded" );

  std::lock_guard<std::mutex> scoped_lock(m_impl_mutex);

  try
    {
      m_impl->create(db_name, includes);
      m_impl->get_superclasses(p_superclasses);
      set_subclasses();
    }
  catch(daq::config::Generic & ex)
    {
      std::ostringstream text;
      text << "failed to create database \'" << db_name << '\'';
      throw ( daq::config::Generic( ERS_HERE, text.str().c_str(), ex ) );
    }
}


bool
Configuration::is_writable(const std::string& db_name) const
{
  if (m_impl == nullptr)
    throw(daq::config::Generic(ERS_HERE, "no implementation loaded" ) );

  std::lock_guard<std::mutex> scoped_lock(m_impl_mutex);

  try
    {
      return m_impl->is_writable(db_name);
    }
  catch(daq::config::Generic & ex)
    {
      std::ostringstream text;
      text << "failed to get write access status for database \'" << db_name<< '\'';
      throw ( daq::config::Generic( ERS_HERE, text.str().c_str(), ex ) );
    }
}


void
Configuration::add_include(const std::string& db_name, const std::string& include)
{
  if (m_impl == nullptr)
    throw daq::config::Generic( ERS_HERE, "no implementation loaded" );

  std::lock_guard<std::mutex> scoped_lock(m_impl_mutex);

  try
    {
      m_impl->add_include(db_name, include);
      m_impl->get_superclasses(p_superclasses);
      set_subclasses();
    }
  catch(daq::config::Generic & ex)
    {
      std::ostringstream text;
      text << "failed to add include \'" << include << "\' to database \'" << db_name<< '\'';
      throw ( daq::config::Generic( ERS_HERE, text.str().c_str(), ex ) );
    }
}

void
Configuration::remove_include(const std::string& db_name, const std::string& include)
{
  if (m_impl == nullptr)
    throw daq::config::Generic( ERS_HERE, "no implementation loaded" );

  std::lock_guard<std::mutex> scoped_lock(m_impl_mutex);
  std::lock_guard<std::mutex> scoped_lock2(m_tmpl_mutex);

  try
    {
      m_impl->remove_include(db_name, include);
      m_impl->get_superclasses(p_superclasses);
      set_subclasses();
    }
  catch(daq::config::Generic & ex)
    {
      std::ostringstream text;
      text << "failed to remove include \'" << include << "\' from database \'" << db_name<< '\'';
      throw ( daq::config::Generic( ERS_HERE, text.str().c_str(), ex ) );
    }
}

void
Configuration::get_includes(const std::string& db_name, std::list<std::string>& includes) const
{
  if (m_impl == nullptr)
    throw daq::config::Generic( ERS_HERE, "no implementation loaded" );

  std::lock_guard<std::mutex> scoped_lock(m_impl_mutex);

  try
    {
      m_impl->get_includes(db_name, includes);
    }
  catch(daq::config::Generic & ex)
    {
      std::ostringstream text;
      text << "failed to get includes of database \'" << db_name<< '\'';
      throw ( daq::config::Generic( ERS_HERE, text.str().c_str(), ex ) );
    }
}


void
Configuration::get_updated_dbs(std::list<std::string>& dbs) const
{
  if (m_impl == nullptr)
    throw daq::config::Generic( ERS_HERE, "no implementation loaded" );

  std::lock_guard<std::mutex> scoped_lock(m_impl_mutex);

  try
    {
      m_impl->get_updated_dbs(dbs);
    }
  catch(daq::config::Generic & ex)
    {
      throw ( daq::config::Generic( ERS_HERE, "get_updated_dbs failed", ex ) );
    }
}


void
Configuration::set_commit_credentials(const std::string& user, const std::string& password)
{
  if (m_impl == nullptr)
    throw daq::config::Generic( ERS_HERE, "no implementation loaded" );

  std::lock_guard<std::mutex> scoped_lock(m_impl_mutex);

  try
    {
      m_impl->set_commit_credentials(user, password);
    }
  catch(daq::config::Generic & ex)
    {
      throw ( daq::config::Generic( ERS_HERE, "set_commit_credentials failed", ex ) );
    }
}


void
Configuration::commit(const std::string& log_message)
{
  ERS_DEBUG(1, "call commit");

  if (m_impl == nullptr)
    throw daq::config::Generic( ERS_HERE, "no implementation loaded");

  std::lock_guard<std::mutex> scoped_lock1(m_tmpl_mutex);  // always lock template objects mutex first
  std::lock_guard<std::mutex> scoped_lock2(m_impl_mutex);

  try
    {
      m_impl->commit(log_message);
    }
  catch (daq::config::Generic & ex)
    {
      throw(daq::config::Generic( ERS_HERE, "commit failed", ex ) );
    }
}

void
Configuration::abort()
{
  ERS_DEBUG(1, "call abort");

  if (m_impl == nullptr)
    throw daq::config::Generic( ERS_HERE, "no implementation loaded");

  std::lock_guard<std::mutex> scoped_lock1(m_tmpl_mutex);  // always lock template objects mutex first
  std::lock_guard<std::mutex> scoped_lock2(m_impl_mutex);

  try
    {
      m_impl->abort();
      _unread_implementation_objects(daq::config::Unknown);
      _unread_template_objects();
      m_impl->get_superclasses(p_superclasses);
      set_subclasses();
    }
  catch (daq::config::Generic & ex)
    {
      throw(daq::config::Generic( ERS_HERE, "abort failed", ex));
    }
}

void
Configuration::prefetch_all_data()
{
  std::lock_guard<std::mutex> scoped_lock1(m_tmpl_mutex);  // always lock template objects mutex first
  std::lock_guard<std::mutex> scoped_lock2(m_impl_mutex);

  try
    {
      m_impl->prefetch_all_data();
    }
  catch (daq::config::Generic & ex)
    {
      throw(daq::config::Generic( ERS_HERE, "prefetch all data failed", ex));
    }
}

void
Configuration::unread_all_objects(bool unread_implementation_objs) noexcept
{
  if (unread_implementation_objs)
    unread_implementation_objects(daq::config::Unknown);

  unread_template_objects();
}


void
Configuration::_unread_template_objects() noexcept
{
  for (auto &j : m_cache_map)
    j.second->m_functions.m_unread_object_fn(j.second);
}

void
Configuration::_unread_implementation_objects(daq::config::ObjectState state) noexcept
{
  for (auto &i : m_impl->m_impl_objects)
    for (auto &j : *i.second)
      {
        std::lock_guard<std::mutex> scoped_lock(j.second->m_mutex);
        j.second->clear();
        j.second->m_state = state;
      }

  for (auto& x : m_impl->m_tangled_objects)
    {
      std::lock_guard<std::mutex> scoped_lock(x->m_mutex);
      x->clear();
      x->m_state = state;
    }
}


void
Configuration::set_subclasses() noexcept
{
  p_subclasses.clear();

  for (const auto &i : p_superclasses)
    for (const auto &j : i.second)
      p_subclasses[j].insert(i.first);
}


//////////////////////////////////////////////////////////////////////////////////////////

  //
  // Test, create and destroy object methods
  //

//////////////////////////////////////////////////////////////////////////////////////////

bool
Configuration::test_object(const std::string& class_name, const std::string& id, unsigned long rlevel, const std::vector<std::string> * rclasses)
{
  try
    {
      std::lock_guard<std::mutex> scoped_lock(m_impl_mutex);
      return m_impl->test_object(class_name, id, rlevel, rclasses);
    }
  catch (daq::config::Generic& ex)
    {
      std::ostringstream text;
      text << "failed to test existence of object \'" << id << '@' << class_name << '\'';
      throw daq::config::Generic( ERS_HERE, text.str().c_str(), ex );
    }
}

void
Configuration::create(const std::string& at, const std::string& class_name, const std::string& id, ConfigObject& object)
{
  try
    {
      std::lock_guard<std::mutex> scoped_lock(m_impl_mutex);
      m_impl->create(at, class_name, id, object);
    }
  catch (daq::config::Generic& ex)
    {
      std::ostringstream text;
      text << "failed to create object \'" << id << '@' << class_name << '\'';
      throw daq::config::Generic( ERS_HERE, text.str().c_str(), ex );
    }
}

void
Configuration::create(const ConfigObject& at, const std::string& class_name, const std::string& id, ConfigObject& object)
{
  try
    {
      std::lock_guard<std::mutex> scoped_lock(m_impl_mutex);
      m_impl->create(at, class_name, id, object);
    }
  catch (daq::config::Generic& ex)
    {
      std::ostringstream text;
      text << "failed to create object \'" << id << '@' << class_name << '\'';
      throw daq::config::Generic( ERS_HERE, text.str().c_str(), ex );
    }
}


void
Configuration::destroy_obj(ConfigObject& object)
{
  try
    {
      std::lock_guard<std::mutex> scoped_lock(m_impl_mutex);
      std::lock_guard<std::mutex> scoped_lock2(m_tmpl_mutex);
      m_impl->destroy(object);
    }
  catch (daq::config::Generic& ex)
    {
      std::ostringstream text;
      text << "failed to destroy object \'" << object << '\'';
      throw daq::config::Generic( ERS_HERE, text.str().c_str(), ex );
    }
}


void
Configuration::rename_object(ConfigObject& obj, const std::string& new_id)
{
  std::lock_guard<std::mutex> scoped_impl_lock(m_tmpl_mutex);  // always lock template objects mutex first
  std::lock_guard<std::mutex> scoped_tmpl_lock(m_impl_mutex);

  std::lock_guard<std::mutex> scoped_obj_lock(obj.m_impl->m_mutex);

  const std::string old_id(obj.m_impl->m_id);

  obj.m_impl->throw_if_deleted();
  obj.m_impl->rename(new_id);
  obj.m_impl->m_id = new_id;
  m_impl->rename_impl_object(obj.m_impl->m_class_name, old_id, new_id);

  ERS_DEBUG(3, " * call rename \'" << old_id << "\' to \'" << new_id << "\' in class \'" << obj.class_name() << "\')");

  config::fmap<CacheBase*>::iterator j = m_cache_map.find(&obj.class_name());
  if (j != m_cache_map.end())
    j->second->m_functions.m_rename_object_fn(j->second, old_id, new_id);

  config::fmap<config::fset>::const_iterator sc = p_superclasses.find(&obj.class_name());

  if (sc != p_superclasses.end())
    for (config::fset::const_iterator c = sc->second.begin(); c != sc->second.end(); ++c)
      {
        config::fmap<CacheBase*>::iterator j = m_cache_map.find(*c);

        if (j != m_cache_map.end())
          j->second->m_functions.m_rename_object_fn(j->second, old_id, new_id);
      }
}



//////////////////////////////////////////////////////////////////////////////////////////

  //
  // Meta-information access methods
  //

//////////////////////////////////////////////////////////////////////////////////////////

const daq::config::class_t&
Configuration::get_class_info(const std::string& class_name, bool direct_only)
{
  std::lock_guard<std::mutex> scoped_lock(m_impl_mutex);

  config::map<daq::config::class_t *>& d_cache(direct_only ? p_direct_classes_desc_cache : p_all_classes_desc_cache);

  config::map<daq::config::class_t *>::const_iterator i = d_cache.find(class_name);

  if (i != d_cache.end())
    return *(i->second);

  try
    {
      daq::config::class_t * d = m_impl->get(class_name, direct_only);
      d_cache[class_name] = d;
      return *d;
    }
  // catch Generic exception only; the NotFound is forwarded from implementation
  catch (daq::config::Generic& ex)
    {
      std::ostringstream text;
      text << "failed to get description of class \'" << class_name << '\'';
      throw daq::config::Generic( ERS_HERE, text.str().c_str(), ex );
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

static void
init_regex(std::unique_ptr<std::regex>& ptr, const std::string& str, const char * what)
{
  if (!str.empty())
    try
      {
        ptr = std::make_unique<std::regex>(str);
      }
    catch (const std::regex_error &ex)
      {
        std::ostringstream text;
        text << "failed to create " << what << " regex \"" << str << "\": " << ex.what();
        throw daq::config::Generic( ERS_HERE, text.str().c_str());
      }
}


template<class T>
static void
add_array_item(boost::property_tree::ptree &pt, const T &val)
{
  boost::property_tree::ptree child;
  child.put("", val);
  pt.push_back(std::make_pair("", child));
}

void
Configuration::export_schema(boost::property_tree::ptree& pt, const std::string& classes_str, bool direct_only)
{
  std::unique_ptr<std::regex> classes_regex;

  init_regex(classes_regex, classes_str, "classes");

  auto cmp_str_ptr = [](const std::string * s1, const std::string * s2) { return *s1 < *s2; };
  std::set<const std::string *, decltype(cmp_str_ptr)> sorted_classes(cmp_str_ptr);

  for (const auto &c : superclasses())
    if (classes_str.empty() || std::regex_match(*c.first, *classes_regex.get()))
      sorted_classes.insert(c.first);

  for (const auto &c : sorted_classes)
    if (classes_str.empty() || std::regex_match(*c, *classes_regex.get()))
      {
        const daq::config::class_t& info(get_class_info(*c, direct_only));

        boost::property_tree::ptree class_pt;

        class_pt.put("abstract", info.p_abstract);
        if (!info.p_description.empty())
          class_pt.put("description", info.p_description);

        if (!info.p_superclasses.empty())
          {
            boost::property_tree::ptree superclasses;

            for (const auto &x : info.p_superclasses)
              add_array_item(superclasses, x);

            class_pt.add_child("superclasses", superclasses);
          }

        if (!info.p_attributes.empty())
          {
            boost::property_tree::ptree attributes;

            for (const auto &x : info.p_attributes)
              {
                boost::property_tree::ptree attribute;

                attribute.put("type", daq::config::attribute_t::type(x.p_type));
                if (!x.p_range.empty())
                  attribute.put("range", x.p_range);
                if (x.p_int_format != daq::config::na_int_format)
                  attribute.put("format", daq::config::attribute_t::format2str(x.p_int_format));
                if (x.p_is_not_null)
                  attribute.put("is-not-null", x.p_is_not_null);
                if (x.p_is_multi_value)
                  attribute.put("is-multi-value", x.p_is_multi_value);
                if (!x.p_default_value.empty())
                  attribute.put("default-value", x.p_default_value);
                if (!x.p_description.empty())
                  attribute.put("description", x.p_description);

                attributes.push_back(boost::property_tree::ptree::value_type(x.p_name, attribute));
              }

            class_pt.add_child("attributes", attributes);
          }

        if (!info.p_relationships.empty())
          {
            boost::property_tree::ptree relationships;

            for (const auto &x : info.p_relationships)
              {
                boost::property_tree::ptree relationship;

                relationship.put("type", x.p_type);
                relationship.put("cardinality", daq::config::relationship_t::card2str(x.p_cardinality));
                if (!x.p_is_aggregation)
                  relationship.put("is-aggregation", x.p_is_aggregation);
                if (!x.p_description.empty())
                  relationship.put("description", x.p_description);

                relationships.push_back(boost::property_tree::ptree::value_type(x.p_name, relationship));
              }

            class_pt.add_child("relationships", relationships);
          }

        pt.put_child(boost::property_tree::ptree::path_type(*c), class_pt);
      }
}

template<class T>
static void
add_data(boost::property_tree::ptree &pt, const ConfigObject &obj, const daq::config::attribute_t &attribute, const std::string &empty_array_item)
{
  auto &o = const_cast<ConfigObject&>(obj);
  if (!attribute.p_is_multi_value)
    {
      T val;
      const_cast<ConfigObject&>(obj).get(attribute.p_name, val);
      pt.put(attribute.p_name, val);
    }
  else
    {
      std::vector<T> values;
      o.get(attribute.p_name, values);

      boost::property_tree::ptree children;

      if (!values.empty())
        for (const auto &v : values)
          add_array_item(children, v);

      else if (!empty_array_item.empty())
        add_array_item(children, empty_array_item);

      pt.add_child(attribute.p_name, children);
    }
}

static void
add_data(boost::property_tree::ptree &pt, const ConfigObject &obj, const daq::config::relationship_t &relationship, const std::string &empty_array_item)
{
  if (relationship.p_cardinality == daq::config::zero_or_many || relationship.p_cardinality == daq::config::one_or_many)
    {
      std::vector<ConfigObject> values;
      const_cast<ConfigObject&>(obj).get(relationship.p_name, values);

      boost::property_tree::ptree children;

      if (!values.empty())
        for (const auto &v : values)
          add_array_item(children, v.full_name());

      else if (!empty_array_item.empty())
        add_array_item(children, empty_array_item);

      pt.add_child(relationship.p_name, children);
    }
  else
    {
      ConfigObject val;
      const_cast<ConfigObject&>(obj).get(relationship.p_name, val);
      pt.put(relationship.p_name, !val.is_null() ? val.full_name() : "");
    }
}

void
Configuration::export_data(boost::property_tree::ptree& pt, const std::string& classes_str, const std::string& objects_str, const std::string& files_str, const std::string& empty_array_item)
{
  std::unique_ptr<std::regex> classes_regex, objects_regex, files_regex;

  init_regex(classes_regex, classes_str, "classes");
  init_regex(objects_regex, objects_str, "objects");
  init_regex(files_regex, files_str, "files");

  auto cmp_str_ptr = [](const std::string * s1, const std::string * s2) { return *s1 < *s2; };
  std::set<const std::string *, decltype(cmp_str_ptr)> sorted_classes(cmp_str_ptr);

  for (const auto &c : superclasses())
    if (classes_str.empty() || std::regex_match(*c.first, *classes_regex.get()))
      sorted_classes.insert(c.first);

  for (const auto &c : sorted_classes)
    if (classes_str.empty() || std::regex_match(*c, *classes_regex.get()))
      {
        const daq::config::class_t& info(get_class_info(*c));

        boost::property_tree::ptree pt_objects;

        std::vector<ConfigObject> objects;
        get(*c, objects);

        auto comp_obj_ptr = [](const ConfigObject * o1, const ConfigObject * o2) { return o1->UID() < o2->UID(); };
        std::set<const ConfigObject *, decltype(comp_obj_ptr)> sorted_objects(comp_obj_ptr);

        for (const auto& x : objects)
          if (objects_str.empty() || std::regex_match(x.UID(), *objects_regex.get()))
            if (x.class_name() == *c)
              if(files_str.empty() || std::regex_match(x.contained_in(), *files_regex.get()))
                sorted_objects.insert(&x);

        if (!sorted_objects.empty())
          {
            boost::property_tree::ptree pt_objects;

            for (const auto& x : sorted_objects)
              {
                boost::property_tree::ptree data;

                for (const auto &a : info.p_attributes)
                  switch (a.p_type)
                    {
                      case daq::config::bool_type:
                              add_data<bool>(data, *x, a, empty_array_item);
                              break;
                      case daq::config::s8_type:
                              add_data<int8_t>(data, *x, a, empty_array_item);
                              break;
                      case daq::config::u8_type:
                              add_data<uint8_t>(data, *x, a, empty_array_item);
                              break;
                      case daq::config::s16_type:
                              add_data<int16_t>(data, *x, a, empty_array_item);
                              break;
                      case daq::config::u16_type:
                              add_data<uint16_t>(data, *x, a, empty_array_item);
                              break;
                      case daq::config::s32_type:
                              add_data<int32_t>(data, *x, a, empty_array_item);
                              break;
                      case daq::config::u32_type:
                              add_data<uint32_t>(data, *x, a, empty_array_item);
                              break;
                      case daq::config::s64_type:
                              add_data<int64_t>(data, *x, a, empty_array_item);
                              break;
                      case daq::config::u64_type:
                              add_data<uint64_t>(data, *x, a, empty_array_item);
                              break;
                      case daq::config::float_type:
                              add_data<float>(data, *x, a, empty_array_item);
                              break;
                      case daq::config::double_type:
                              add_data<double>(data, *x, a, empty_array_item);
                              break;
                      case daq::config::date_type:
                      case daq::config::time_type:
                      case daq::config::enum_type:
                      case daq::config::class_type:
                      case daq::config::string_type:
                              add_data<std::string>(data, *x, a, empty_array_item);
                              break;
                      default:
                              throw std::runtime_error("Invalid type of attribute " + a.p_name);

                    }

                for (const auto &r : info.p_relationships)
                  add_data(data, *x, r, empty_array_item);

                pt_objects.push_back(boost::property_tree::ptree::value_type(x->UID(), data));
              }

            pt.put_child(boost::property_tree::ptree::path_type(*c), pt_objects);
          }
      }
}

//////////////////////////////////////////////////////////////////////////////////////////

  //
  // Methods to get versions
  //

//////////////////////////////////////////////////////////////////////////////////////////


std::vector<daq::config::Version>
Configuration::get_changes()
{
  try
    {
      std::lock_guard<std::mutex> scoped_lock(m_impl_mutex);
      return m_impl->get_changes();
    }
  catch (daq::config::Generic& ex)
    {
      throw daq::config::Generic( ERS_HERE, "failed to get new versions", ex );
    }
}


std::vector<daq::config::Version>
Configuration::get_versions(const std::string& since, const std::string& until, daq::config::Version::QueryType type, bool skip_irrelevant)
{
  try
    {
      std::lock_guard<std::mutex> scoped_lock(m_impl_mutex);
      return m_impl->get_versions(since, until, type, skip_irrelevant);
    }
  catch (daq::config::Generic& ex)
    {
      throw daq::config::Generic( ERS_HERE, "failed to get versions", ex );
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

  //
  // Subscription and notification methods
  //

//////////////////////////////////////////////////////////////////////////////////////////


  // The methods checks that given callback exists

Configuration::CallbackSubscription *
Configuration::find_callback(CallbackId cb_handler) const
{
  return ((!cb_handler || (m_callbacks.find(cb_handler) == m_callbacks.end())) ? 0 : cb_handler);
}


Configuration::CallbackId
Configuration::subscribe(const ::ConfigurationSubscriptionCriteria& criteria, notify user_cb, void * parameter)
{
  // check if there is no subscription function provided

  if (!user_cb)
    {
      throw daq::config::Generic( ERS_HERE, "callback function is not defined" );
    }

  // create callback subscription structure

  ::Configuration::CallbackSubscription * cs = new CallbackSubscription();

  cs->m_criteria = criteria;
  cs->m_cb = user_cb;
  cs->m_param = parameter;

  // FIXME: bug in OksConfiguration subscribe() with enter_loop=true
  std::lock_guard<std::mutex> scoped_lock(m_else_mutex);// TEST 2010-02-03

  m_callbacks.insert(cs);

  try
    {
      reset_subscription();
      return cs;
    }
  catch (daq::config::Generic& ex)
    {
      m_callbacks.erase(cs);
      delete cs;
      throw daq::config::Generic( ERS_HERE, "subscription failed", ex );
    }
}

Configuration::CallbackId
Configuration::subscribe(pre_notify user_cb, void * parameter)
{
  // check if there is no subscription function provided

  if (!user_cb)
    throw(daq::config::Generic( ERS_HERE, "callback function is not defined" ) );

  // create callback subscription structure

  CallbackPreSubscription * cs = new CallbackPreSubscription();

  cs->m_cb = user_cb;
  cs->m_param = parameter;

  {
    std::lock_guard<std::mutex> scoped_lock(m_else_mutex);
    m_pre_callbacks.insert(cs);
  }

  return reinterpret_cast<CallbackSubscription *>(cs);
}


void
Configuration::unsubscribe(CallbackId id)
{
  std::lock_guard < std::mutex > scoped_lock(m_else_mutex);

  if (id)
    {
      CallbackSet::iterator i = m_callbacks.find(id);
      PreCallbackSet::iterator j = m_pre_callbacks.find(reinterpret_cast<CallbackPreSubscription *>(id));

      if (i != m_callbacks.end())
        {
          delete id;
          m_callbacks.erase(i);
        }
      else if (j != m_pre_callbacks.end())
        {
          delete reinterpret_cast<CallbackPreSubscription *>(id);
          m_pre_callbacks.erase(j);
        }
      else
        {
          std::ostringstream text;
          text << "unsubscription failed for CallbackId = " << (void *) id << " (no such callback id found)";
          throw(daq::config::Generic( ERS_HERE, text.str().c_str() ) );
        }
    }
  else
    {
      for (auto &i : m_callbacks)
        delete i;

      for (auto &i : m_pre_callbacks)
        delete i;

      m_callbacks.clear();
      m_pre_callbacks.clear();
    }

  try
    {
      reset_subscription();
    }
  catch (daq::config::Generic& ex)
    {
      throw daq::config::Generic( ERS_HERE, "unsubscription failed", ex );
    }
}

void
Configuration::reset_subscription()
{
  // check that there is no at least one subscription
  // if NO, then unsubscribe

  if (m_callbacks.empty())
    {
      m_impl->unsubscribe();
      return;
    }

  // prepare subscription criteria

  ::ConfigurationSubscriptionCriteria::ObjectMap obj_subscriptions;
  std::set<std::string> class_subscriptions;

  // among existing subscriptions find one who has all subscriptions

  bool found_subscribe_all = false;

  for (const auto &i : m_callbacks)
    if (i->m_criteria.get_classes_subscription().empty() && i->m_criteria.get_objects_subscription().empty())
      {
        found_subscribe_all = true;
        break;
      }

  if (found_subscribe_all == false)
    {
      // build list of all classes for which there is a subscription
      for (const auto &i : m_callbacks)
        for (const auto &j : i->m_criteria.get_classes_subscription())
          class_subscriptions.insert(j);

      // build list of all objects for which there is a subscription (if there is no such class)
      for (const auto &i : m_callbacks)
        for (const auto &j : i->m_criteria.get_objects_subscription())
          {
            const std::string &obj_class_name = j.first;
            if (class_subscriptions.find(obj_class_name) == class_subscriptions.end())
              for (const auto &k : j.second)
                obj_subscriptions[obj_class_name].insert(k);
          }
    }

  m_impl->subscribe(class_subscriptions, obj_subscriptions, system_cb, system_pre_cb);
}


void
Configuration::update_impl_objects(config::pmap<config::map<ConfigObjectImpl *> * >& cache, ConfigurationChange& change, const std::string * class_name)
{
  if (change.get_removed_objs().empty() == false)
    {
      config::pmap<config::map<ConfigObjectImpl *> *>::iterator i = cache.find(class_name);

      if (i != cache.end())
        {
          for (auto & x : change.get_removed_objs())
            {
              config::map<ConfigObjectImpl *>::iterator j = i->second->find(x);
              if (j != i->second->end())
                {
                  ERS_DEBUG( 2 , "set implementation object " << x << '@' << *class_name << " [" << (void *)j->second << "] deleted");

                  std::lock_guard<std::mutex> scoped_lock(j->second->m_mutex);
                  j->second->m_state = daq::config::Deleted;
                  j->second->clear();
                }
            }
        }
    }

  if (change.get_created_objs().empty() == false)
    {
      config::pmap<config::map<ConfigObjectImpl *> *>::iterator i = cache.find(class_name);

      if (i != cache.end())
        {
          for (auto & x : change.get_created_objs())
            {
              config::map<ConfigObjectImpl *>::iterator j = i->second->find(x);
              if (j != i->second->end())
                {
                  ERS_DEBUG( 2 , "re-set created implementation object " << x << '@' << *class_name << " [" << (void *)j->second << ']');

                  std::lock_guard<std::mutex> scoped_lock(j->second->m_mutex);
                  j->second->reset(); // it does not matter what the state was, always reset
                }
            }
        }
    }

  if (change.get_modified_objs().empty() == false)
    {
      config::pmap<config::map<ConfigObjectImpl *> *>::iterator i = cache.find(class_name);

      if (i != cache.end())
        {
          for (auto & x : change.get_modified_objs())
            {
              config::map<ConfigObjectImpl *>::iterator j = i->second->find(x);
              if (j != i->second->end())
                {
                  ERS_DEBUG(2, "clear implementation object " << x << '@' << *class_name << " [" << (void *)j->second << ']');

                  std::lock_guard<std::mutex> scoped_lock(j->second->m_mutex);

                  if(j->second->m_state != daq::config::Valid)
                    j->second->reset();
                  else
                    j->second->clear();
                }
            }
        }
    }
}


  // note, the std::lock_guard<std::mutex> scoped_lock(conf->m_tmpl_mutex) is already set by caller

void
Configuration::update_cache(std::vector<ConfigurationChange *>& changes) noexcept
{
  ERS_DEBUG(3, "*** Enter Configuration::update_cache() with changes:\n" << changes);

  // Remove deleted and update modified implementation objects first
  for (const auto& i : changes)
    {
      const std::string * class_name = &DalFactory::instance().get_known_class_name_ref(i->get_class_name());

      update_impl_objects(m_impl->m_impl_objects, *i, class_name);

      // delete/update implementation objects defined in superclasses
      config::fmap<config::fset>::const_iterator sc = p_superclasses.find(class_name);

      if (sc != p_superclasses.end())
        for (const auto &c : sc->second)
          update_impl_objects(m_impl->m_impl_objects, *i, c);

      // delete/update implementation objects defined in subclasses
      sc = p_subclasses.find(class_name);

      if (sc != p_subclasses.end())
        for (const auto &c : sc->second)
          update_impl_objects(m_impl->m_impl_objects, *i, c);
    }

  for (const auto& i : changes)
    {
      const std::string * class_name = &DalFactory::instance().get_known_class_name_ref(i->get_class_name()); // FIXME: optimise with above

      // invoke configuration update if there are template objects of given class

        {
          config::fmap<CacheBase*>::iterator j = m_cache_map.find(class_name);

          if (j != m_cache_map.end())
            {
              ERS_DEBUG(3, " * call update on \'" << j->first << "\' template objects");
              j->second->m_functions.m_update_fn(*this, i);
            }
        }


      // invoke configuration update if there are template objects in super-classes

        {
          config::fmap<config::fset>::const_iterator sc = p_superclasses.find(class_name);

          if (sc != p_superclasses.end())
            {
              for (const auto& c : sc->second)
                {
                  config::fmap<CacheBase*>::iterator j = m_cache_map.find(c);

                  if (j != m_cache_map.end())
                    {
                      ERS_DEBUG(3, " * call update on \'" << j->first << "\' template objects (as super-class of \'" << *class_name << "\')");
                      j->second->m_functions.m_update_fn(*this, i);
                    }
                }
            }
        }


      // invoke configuration update if there are template objects in sub-classes

        {
          config::fmap<config::fset>::const_iterator sc = p_subclasses.find(class_name);

          if (sc != p_subclasses.end())
            {
              for (const auto& c : sc->second)
                {
                  config::fmap<CacheBase*>::iterator j = m_cache_map.find(c);

                  if (j != m_cache_map.end())
                    {
                      ERS_DEBUG(3, " * call update on \'" << j->first << "\' template objects (as sub-class of \'" << *class_name << "\')");
                      j->second->m_functions.m_update_fn(*this, i);
                    }
                }
            }
        }

    }

}


void
Configuration::system_cb(std::vector<ConfigurationChange *>& changes, Configuration * conf) noexcept
{

  ERS_DEBUG(3,
    "*** Enter Configuration::system_cb()\n"
    "*** Number of user subscriptions: " << conf->m_callbacks.size()
  );

  // call config actions if any
  {
    std::lock_guard<std::mutex> scoped_lock(conf->m_impl_mutex);
    std::lock_guard<std::mutex> scoped_lock2(conf->m_actn_mutex);
    for (auto & i : conf->m_actions)
      i->notify(changes);
  }


  // update template objects in cache
  {
    std::lock_guard<std::mutex> scoped_lock(conf->m_tmpl_mutex);  // always lock template objects mutex first
    std::lock_guard<std::mutex> scoped_lock2(conf->m_impl_mutex);
    conf->update_cache(changes);
  }


  // user removed all subscriptions
  if(conf->m_callbacks.empty()) return;

  // note, one cannot lock m_tmpl_mutex or m_impl_mutex here,
  // since user callback may call arbitrary get() methods to access config
  // and template objects locking above two mutexes
  std::lock_guard<std::mutex> scoped_lock(conf->m_else_mutex);

  // check if there is only one subscription
  if (conf->m_callbacks.size() == 1)
    {
      auto j = *conf->m_callbacks.begin();
      (*(j->m_cb))(changes, j->m_param);
    }
  // may need to calculate the changes for each subscription
  else
    {
      for (const auto &j : conf->m_callbacks)
        {

          if (ers::debug_level() >= 3)
            {
              std::ostringstream text;

              text <<
                  "*** Process subscription " << (void*) j << "\n"
                  " class subscription done for " << j->m_criteria.get_classes_subscription().size() << " classes:\n";

              for (const auto &i1 : j->m_criteria.get_classes_subscription())
                text << " * class \"" << i1 << "\"\n";

              text << " object subscription done in " << j->m_criteria.get_objects_subscription().size() << " classes:\n";

              for (const auto &i2 : j->m_criteria.get_objects_subscription())
                {
                  text << " * class \"" << (i2.first) << "\":\n";
                  for (const auto &i3 : i2.second)
                    text << "  - \"" << i3 << "\"\n";
                }

              ERS_DEBUG(3, text.str());
            }

          if (j->m_criteria.get_classes_subscription().empty() && j->m_criteria.get_objects_subscription().empty())
            {
              try
                {
                  ERS_DEBUG(3, "*** Invoke callback " << (void *)j << " with\n" << changes);
                  (*(j->m_cb))(changes, j->m_param);
                }
              catch (const ers::Issue &ex)
                {
                  ers::error(daq::config::Generic( ERS_HERE, "user callback thrown ers exception", ex));
                }
              catch (const std::exception &ex)
                {
                  ers::error(daq::config::Generic( ERS_HERE, "user callback thrown std exception", ex));
                }
              catch (...)
                {
                  ers::error(daq::config::Generic( ERS_HERE, "user callback thrown unknown exception"));
                }
            }
          else
            {
              std::vector<ConfigurationChange*> changes1;

              for (const auto &i : changes)
                {
                  const std::string &cname = i->get_class_name();
                  ConfigurationChange *class_changes = nullptr;

                  ::ConfigurationSubscriptionCriteria::ObjectMap::const_iterator p = j->m_criteria.get_objects_subscription().find(cname);
                  const bool found_obj_subscription(p != j->m_criteria.get_objects_subscription().end());
                  const bool found_class_subscription(j->m_criteria.get_classes_subscription().find(cname) != j->m_criteria.get_classes_subscription().end());

                  if (found_class_subscription || found_obj_subscription)
                    class_changes = new ConfigurationChange(cname);

                  if (found_class_subscription)
                    {
                      for (const auto &k : i->m_modified)
                        class_changes->m_modified.push_back(k);

                      for (const auto &k : i->m_created)
                        class_changes->m_created.push_back(k);

                      for (const auto &k : i->m_removed)
                        class_changes->m_removed.push_back(k);
                    }

                  if (found_obj_subscription)
                    {
                      for (const auto &obj_id : i->m_modified)
                        if (p->second.find(obj_id) != p->second.end())
                          class_changes->m_modified.push_back(obj_id);

                      for (const auto &obj_id : i->m_removed)
                        if (p->second.find(obj_id) != p->second.end())
                          class_changes->m_removed.push_back(obj_id);
                    }

                  // the changes for given class can be empty if there is subscription on objects of given class, but no such objects were modified
                  if (class_changes)
                    {
                      if (class_changes->m_modified.empty() && class_changes->m_created.empty() && class_changes->m_removed.empty())
                        delete class_changes;
                      else
                        changes1.push_back(class_changes);
                    }
                }

              if (!changes1.empty())
                {
                  ERS_DEBUG(3, "*** Invoke callback " << (void *)j << " with\n" << changes1);

                  try
                    {
                      (*(j->m_cb))(changes1, j->m_param);
                    }
                  catch (const ers::Issue &ex)
                    {
                      ers::error(daq::config::Generic( ERS_HERE, "user callback thrown ers exception", ex));
                    }
                  catch (const std::exception &ex)
                    {
                      ers::error(daq::config::Generic( ERS_HERE, "user callback thrown std exception", ex));
                    }
                  catch (...)
                    {
                      ers::error(daq::config::Generic( ERS_HERE, "user callback thrown unknown exception"));
                    }

                  for (const auto &i : changes1)
                    delete i;
                }
            }
        }
    }

  ERS_DEBUG(3,"*** Leave Configuration::system_cb()");
}


void
Configuration::system_pre_cb(Configuration * conf) noexcept
{
  ERS_DEBUG(3,"*** Enter Configuration::system_pre_cb()");

  std::lock_guard<std::mutex> scoped_lock(conf->m_else_mutex);

  for(auto& j : conf->m_pre_callbacks)
    {
      ERS_DEBUG(3, "*** Invoke callback " << (void *)(j));
      (*(j->m_cb))(j->m_param);
    }

  ERS_DEBUG(3,"*** Leave Configuration::system_pre_cb()");
}


void
ConfigurationChange::add(std::vector<ConfigurationChange*> &changes, const std::string &class_name, const std::string &obj_name, const char action)
{
  ConfigurationChange *class_changes = nullptr;

  for (const auto &c : changes)
    if (class_name == c->get_class_name())
      {
        class_changes = c;
        break;
      }

  if (!class_changes)
    {
      class_changes = new ConfigurationChange(class_name);
      changes.push_back(class_changes);
    }

  std::vector<std::string>& clist = (
    action == '+' ? class_changes->m_created :
    action == '-' ? class_changes->m_removed :
    class_changes->m_modified
  );

  clist.push_back(obj_name);
}


void
ConfigurationChange::clear(std::vector<ConfigurationChange*> &changes)
{
  for (const auto &i : changes)
    delete i;

  changes.clear();
}


static void
print_svect(std::ostream& s, const std::vector<std::string>& v, const char * name)
{
  s << "  * " << v.size() << name;

  for (auto i = v.begin(); i != v.end(); ++i)
    {
      s << ((i == v.begin()) ? ": " : ", ");
      s << '\"' << *i << '\"';
    }

  s << std::endl;
}


std::ostream&
operator<<(std::ostream &s, const ConfigurationChange &c)
{
  s << " changes for class \'" << c.get_class_name() << "\' include:\n";

  print_svect(s, c.get_modified_objs(), " modified object(s)");
  print_svect(s, c.get_created_objs(), " created object(s)");
  print_svect(s, c.get_removed_objs(), " removed object(s)");

  return s;
}


std::ostream&
operator<<(std::ostream &s, const std::vector<ConfigurationChange*> &v)
{
  s << "There are configuration changes in " << v.size() << " classes:\n";

  for (const auto &i : v)
    s << *i;

  return s;
}

void
Configuration::print(std::ostream &s) const noexcept
{
  s << "Configuration object:\n  Inheritance Hierarchy (class - all it's superclasses):\n";

  for (const auto &i : p_superclasses)
    {
      s << "  * \'" << *i.first << "\' - ";
      if (i.second.empty())
        s << "(null)";
      else
        for (auto j = i.second.begin(); j != i.second.end(); ++j)
          {
            if (j != i.second.begin())
              s << ", ";
            s << '\'' << **j << '\'';
          }
      s << std::endl;
    }
}

bool
Configuration::try_cast(const std::string& target, const std::string& source) noexcept
{
  return try_cast(&DalFactory::instance().get_known_class_name_ref(target), &DalFactory::instance().get_known_class_name_ref(source));
}

bool
Configuration::try_cast(const std::string *target, const std::string *source) noexcept
{
  if (target == source)
    {
      ERS_DEBUG(2, "cast \'" << *source << "\' => \'" << *target << "\' is allowed (equal classes)");
      return true;
    }

  config::fmap<config::fset>::iterator i = p_superclasses.find(source);

  if (i == p_superclasses.end())
    {
      ERS_DEBUG(2, "cast \'" << *source << "\' => \'" << *target << "\' is not possible (source class is not loaded)");
      return false;
    }

  if (i->second.find(target) != i->second.end())
    {
      ERS_DEBUG(2, "cast \'" << *source << "\' => \'" << *target << "\' is allowed (use inheritance)");
      return true;
    }

  ERS_DEBUG(2, "cast \'" << *source << "\' => \'" << *target << "\' is not allowed (class \'" << *source << "\' has no \'" << *target << "\' as a superclass)");

  return false;
}


std::ostream&
operator<<(std::ostream &s, const Configuration &c)
{
  c.print(s);
  return s;
}

std::ostream&
operator<<(std::ostream& s, const DalObject * obj)
{
  if (obj == nullptr)
    DalObject::p_null(s);
  else if (obj->is_deleted())
    s << "(deleted object " << obj->UID() << '@' << obj->class_name() << ')';
  else
    s << '\'' << obj->UID() << '@' << obj->class_name() << '\'';

  return s;
}

std::string
Configuration::mk_ref_ex_text(const char * what, const std::string& cname, const std::string& rname, const ::ConfigObject& obj) noexcept
{
  std::ostringstream text;
  text << "failed to get " << what << " of class \'" << cname << "\' via relationship \'" << rname << "\' of object \'" << obj << '\'';
  return text.str();
}


std::string
Configuration::mk_ref_by_ex_text(const std::string& cname, const std::string& rname, const ::ConfigObject& obj) noexcept
{
  std::ostringstream text;
  text << "failed to get objects of class \'" << cname << "\' referencing object \'" << obj << "\' via relationship \'" << rname << '\'';
  return text.str();
}

std::vector<const DalObject*>
Configuration::make_dal_objects(std::vector<ConfigObject>& objs, bool upcast_unregistered)
{
  std::vector<const DalObject*> result;

  for (auto &i : objs)
    if (DalObject *o = DalFactory::instance().get(*this, i, i.UID(), upcast_unregistered)) // FIXME: 2018-11-09: pass right UID()
      result.push_back(o);

  return result;
}

const DalObject*
Configuration::make_dal_object(ConfigObject& obj, const std::string& uid, const std::string& class_name)
{
  return DalFactory::instance().get(*this, obj, uid, class_name);
}


std::vector<const DalObject*>
Configuration::referenced_by(const DalObject& obj, const std::string& relationship_name, bool check_composite_only, bool upcast_unregistered, bool init, unsigned long rlevel, const std::vector<std::string> * rclasses)
{
  try
    {
      std::vector<ConfigObject> objs;
      std::lock_guard<std::mutex> scoped_lock(m_tmpl_mutex);

      obj.p_obj.referenced_by(objs, relationship_name, check_composite_only, rlevel, rclasses);
      return make_dal_objects(objs, upcast_unregistered);
    }
  catch (daq::config::Generic & ex)
    {
      throw(daq::config::Generic( ERS_HERE, mk_ref_by_ex_text("DalObject", relationship_name, obj.p_obj).c_str(), ex ) );
    }
}

bool
DalObject::get_rel_objects(const std::string &name, bool upcast_unregistered, std::vector<const DalObject*> &objs) const
{
  std::vector<ConfigObject> c_objs;

  if (const_cast<ConfigObject*>(&p_obj)->rel(name, c_objs))
    {
      std::lock_guard<std::mutex> scoped_lock(p_db.m_tmpl_mutex);
      p_db.make_dal_objects(c_objs, upcast_unregistered).swap(objs);
      return true;
    }

  return false;
}


bool
DalObject::get_algo_objects(const std::string &name, std::vector<const DalObject*> &objs) const
{
  const std::string &suitable_dal_class = DalFactory::instance().class4algo(p_db, class_name(), name);

  ERS_DEBUG(2, "suitable class for algorithm " << name << " on object " << this << " is " << suitable_dal_class);

  if (!suitable_dal_class.empty())
    if (const DalObject *obj = p_db.make_dal_object(const_cast<ConfigObject&>(p_obj), UID(), suitable_dal_class))
      {
        obj->get(name, false).swap(objs);
        return true;
      }

  return false;
}



  // DalObject helper

void
DalObject::p_null(std::ostream &s)
{
  s << "(null)";
}

void
DalObject::p_rm(std::ostream &s)
{
  s << "(deleted object)";
}

void
DalObject::p_error(std::ostream &s, daq::config::Exception &ex)
{
  s << "ERROR in generated DAL print method:\n\twas caused by: " << ex << std::endl;
}

void
DalObject::p_hdr(std::ostream &s, unsigned int indent, const std::string &cl, const char *nm) const
{
  const std::string str(indent, ' ');
  s << str;
  if (nm)
    s << nm << ' ';
  s << cl << " object:\n" << str << "  id: \'" << UID() << "\', class name: \'" << DalObject::class_name() << "\'\n";
}

namespace config
{
  void
  p_sv_rel(std::ostream &s, const std::string &str, const std::string &name, const DalObject *obj)
  {
    s << str << name << ": " << obj << '\n';
  }
}


void DalObject::throw_init_ex(daq::config::Exception& ex)
{
  std::ostringstream text;
  text << "failed to init " << this << ":\n\twas caused by: " << ex << std::endl;
  p_was_read = false;
  throw daq::config::Generic (ERS_HERE, text.str().c_str());
}

void DalObject::throw_get_ex(const std::string& what, const std::string& class_name, const DalObject * obj)
{
  std::ostringstream text;
  text << "cannot find relationship or algorithm \"" << what << "\" in c++ class \"" << class_name << "\" for object " << obj;
  throw daq::config::Generic(ERS_HERE, text.str().c_str());
}


DalFactory &
DalFactory::instance()
{
  static DalFactory * instance = ers::SingletonCreator<DalFactory>::create();
  return *instance;
}


DalObject *
DalFactory::get(Configuration& db, ConfigObject& obj, const std::string& uid, bool upcast_unregistered) const
{
  return (*instance().functions(db, obj.class_name(), upcast_unregistered).m_creator_fn)(db, obj, uid);
}

DalObject *
DalFactory::get(Configuration& db, ConfigObject& obj, const std::string& uid, const std::string& class_name) const
{
  return (*instance().functions(db, class_name, false).m_creator_fn)(db, obj, uid);
}


const DalFactoryFunctions&
DalFactory::functions(const Configuration& db, const std::string& name, bool upcast_unregistered) const
{
  auto it = m_classes.find(name);

  if (it == m_classes.end())
    {
      if (upcast_unregistered)
        {
          auto x = db.superclasses().find(&name);
          if (x != db.superclasses().end())
            {
              for (auto c : x->second)
                {
                  auto sc = m_classes.find(*c);
                  if (sc != m_classes.end())
                    {
                      ERS_DEBUG(1, "use first suitable base class " << c << " instead of unregistered DAL class " << name);
                      return sc->second;
                    }
                }
            }
        }

      std::string text(std::string("DAL class ") + name + " was not registered");
      throw daq::config::Generic(ERS_HERE, text.c_str());
    }

  return it->second;
}

const std::string&
DalFactory::class4algo(Configuration& db, const std::string& name, const std::string& algorithm) const
{
  for (const auto& x : m_classes)
    if (x.second.m_algorithms.find(algorithm) != x.second.m_algorithms.end() && db.try_cast(x.first, name))
      return x.first;

  static const std::string empty;
  return empty;
}


const DalFactoryFunctions&
DalFactory::functions(const std::string& name) const
{
  auto it = m_classes.find(name);

  ERS_ASSERT_MSG( (it != m_classes.end()), "writer lock was not initialized" );

  return it->second;
}

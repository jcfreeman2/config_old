#include <stdlib.h>

#include "config/Configuration.h"
#include "config/ConfigurationImpl.h"
#include "config/Schema.h"

namespace daq {
  namespace config {

    const char * bool2str(bool value) { return (value ? "yes" : "no"); }

    attribute_t::attribute_t(
      const std::string& name, type_t type, const std::string& range,
      int_format_t int_format, bool is_not_null, bool is_multi_value,
      const std::string& default_value, const std::string& description
    ) :
      p_name             (name),
      p_type             (type),
      p_range            (range),
      p_int_format       (int_format),
      p_is_not_null      (is_not_null),
      p_is_multi_value   (is_multi_value),
      p_default_value    (default_value),
      p_description      (description)
    { ; }

    const char * attribute_t::type2str(type_t type)
    {
      switch(type) {
        case bool_type:   return "boolean";
        case s8_type:     return "8-bits signed integer";
        case u8_type:     return "8-bits unsigned integer";
        case s16_type:    return "16-bits signed integer";
        case u16_type:    return "16-bits unsigned integer";
        case s32_type:    return "32-bits signed integer";
        case u32_type:    return "32-bits unsigned integer";
        case s64_type:    return "64-bits signed integer";
        case u64_type:    return "64-bits unsigned integer";
        case float_type:  return "float";
        case double_type: return "double";
        case date_type:   return "date";
        case time_type:   return "time";
        case string_type: return "string";
        case enum_type:   return "enumeration";
        case class_type:  return "class reference";
	default:          return "unknown";
      }
    }

    const char * attribute_t::type(type_t type)
    {
      switch(type) {
        case bool_type:   return "bool";
        case s8_type:     return "s8";
        case u8_type:     return "u8";
        case s16_type:    return "s16";
        case u16_type:    return "u16";
        case s32_type:    return "s32";
        case u32_type:    return "u32";
        case s64_type:    return "s64";
        case u64_type:    return "u64";
        case float_type:  return "float";
        case double_type: return "double";
        case date_type:   return "date";
        case time_type:   return "time";
        case string_type: return "string";
        case enum_type:   return "enum";
        case class_type:  return "class";
        default:          return "unknown";
      }
    }

    const char * attribute_t::format2str(int_format_t type)
    {
      switch(type) {
        case oct_int_format:   return "octal";
        case dec_int_format:   return "decimal";
        case hex_int_format:   return "hexadecimal";
	default:               return "not applicable";
      }
    }

    void attribute_t::print(std::ostream& out, const std::string& prefix) const
    {
      out
        << prefix << "attribute \'" << p_name << "\'\n"
        << prefix << "  type: \'" << type2str(p_type) << "\'\n"
        << prefix << "  range: \'" << p_range << "\'\n";

      if(p_int_format != na_int_format) {
        out << prefix << "  integer format: \'" << format2str(p_int_format) << "\'\n";
      }

      out
        << prefix << "  is not null: " << bool2str(p_is_not_null) << '\n'
        << prefix << "  is multi-value: " << bool2str(p_is_multi_value) << '\n'
        << prefix << "  default value: \'" << p_default_value << "\'\n"
        << prefix << "  description: \'" << p_description << '\'';
    }
    
    std::ostream& operator<<(std::ostream& out, const attribute_t& a)
    {
      a.print(out);
      return out;
    }


    relationship_t::relationship_t(
      const std::string& name, const std::string& type, bool can_be_null,
      bool is_multi_value, bool is_aggregation, const std::string& description
    ) :
      p_name             (name),
      p_type             (type),
      p_cardinality      (
                            (can_be_null  && !is_multi_value) ? zero_or_one  :
                            (can_be_null  && is_multi_value ) ? zero_or_many :
                            (!can_be_null && is_multi_value ) ? one_or_many  :
	                    only_one
                         ),
      p_is_aggregation   (is_aggregation),
      p_description      (description)
    { ; }
    
    const char * relationship_t::card2str(cardinality_t cardinality)
    {
      switch(cardinality) {
        case zero_or_one:    return "zero or one";
        case zero_or_many:   return "zero or many";
        case only_one:       return "one";
        case one_or_many:    return "one or many";
	default:             return "unknown";
      }
    }

    void relationship_t::print(std::ostream& out, const std::string& prefix) const
    {
      out
        << prefix << "relationship \'" << p_name << "\'\n"
        << prefix << "  class type: \'" << p_type << "\'\n"
        << prefix << "  cardinality: \'" << card2str(p_cardinality) << "\'\n"
        << prefix << "  is aggregation: \'" << bool2str(p_is_aggregation) << "\'\n"
        << prefix << "  description: \'" << p_description << '\'';
    }

    std::ostream& operator<<(std::ostream& out, const relationship_t& r)
    {
      r.print(out);
      return out;
    }

    class_t::class_t(
      const std::string& name,
      const std::string& description,
      bool is_abstract
    ) :
    p_name             (name),
    p_description      (description),
    p_abstract         (is_abstract)
    { ; }

    void class_t::print(std::ostream& out, const std::string& prefix) const
    {
      out
        << prefix << "class \'" << p_name << "\'\n"
        << prefix << "  is abstract: \'" << bool2str(p_abstract) << "\'\n"
        << prefix << "  description: \'" << p_description << "\'\n";

      if(p_superclasses.empty()) {
        out << prefix << "  there are no superclasses\n";
      }
      else {
        out << prefix << "  " << p_superclasses.size() << " superclass(es):\n";
	for(std::vector<std::string>::const_iterator i = p_superclasses.begin(); i != p_superclasses.end(); ++i) {
	  out << prefix << "    \'" << *i << "\'\n";
	}
      }

      if(p_subclasses.empty()) {
        out << prefix << "  there are no subclasses\n";
      }
      else {
        out << prefix << "  " << p_subclasses.size() << " subclass(es):\n";
	for(std::vector<std::string>::const_iterator i = p_subclasses.begin(); i != p_subclasses.end(); ++i) {
	  out << prefix << "    \'" << *i << "\'\n";
	}
      }

      std::string new_prefix(prefix);
      new_prefix += "    ";

      if(p_attributes.empty()) {
        out << prefix << "  there are no attributes\n";
      }
      else {
        out << prefix << "  " << p_attributes.size() << " attribute(s):\n";
	for(std::vector<attribute_t>::const_iterator i = p_attributes.begin(); i != p_attributes.end(); ++i) {
	  (*i).print(out, new_prefix.c_str());
	  out << std::endl;
	}
      }

      if(p_relationships.empty()) {
        out << prefix << "  there are no relationships\n";
      }
      else {
        out << prefix << "  " << p_relationships.size() << " relationship(s):\n";
	for(std::vector<relationship_t>::const_iterator i = p_relationships.begin(); i != p_relationships.end(); ++i) {
	  (*i).print(out, new_prefix.c_str());
	  out << std::endl;
	}
      }
    }

    std::ostream& operator<<(std::ostream& out, const class_t& c)
    {
      c.print(out);
      return out;
    }

  }
}


ConfigurationImpl::ConfigurationImpl() noexcept :
  p_number_of_cache_hits  (0),
  p_number_of_object_read (0),
  m_conf                  (0)
{
}

ConfigurationImpl::~ConfigurationImpl()
{
  clean();
}

void
ConfigurationImpl::print_cache_info() noexcept
{
  std::cout <<
    "Configuration implementation profiler report:\n"
    "  number of read objects: " << p_number_of_object_read << "\n"
    "  number of cache hits: " << p_number_of_cache_hits << std::endl;
}


#ifndef ERS_NO_DEBUG
# define CONFIG_ADD_DEBUG_MSG(STREAM, MSG) if(ers::debug_level() >= 4) { *STREAM << MSG; }
#else
# define CONFIG_ADD_DEBUG_MSG(STREAM, MSG) ;
#endif


ConfigObjectImpl *
ConfigurationImpl::get_impl_object(const std::string& name, const std::string& id) const noexcept
{
#ifndef ERS_NO_DEBUG
  std::unique_ptr<std::ostringstream> dbg_text;
  if(ers::debug_level() >= 4)
    dbg_text.reset(new std::ostringstream());
#endif

  config::pmap<config::map<ConfigObjectImpl *> *>::const_iterator i = m_impl_objects.find(&name);

  const std::string * class_name = nullptr;

  if(i != m_impl_objects.end()) {
    config::map<ConfigObjectImpl *>::const_iterator j = i->second->find(id);

    if(j != i->second->end()) {
      p_number_of_cache_hits++;
      ERS_DEBUG(4, "\n  * found the object with id = \'" << id << "\' in class \'" << name << '\'' );
      return j->second;
    }

    class_name = i->first;

#ifndef ERS_NO_DEBUG

      // prepare and print out debug message

    if(ers::debug_level() >= 4) {
      CONFIG_ADD_DEBUG_MSG( dbg_text , "\n  * there is no object with id = \'" << id << "\' found in the class \'" << name << "\' that has " << i->second->size() << " objects in cache: " )
      for(j=i->second->begin(); j != i->second->end();++j) {
        if(j != i->second->begin()) { CONFIG_ADD_DEBUG_MSG( dbg_text , ", " ) }
	CONFIG_ADD_DEBUG_MSG( dbg_text , '\'' << j->first << '\'' )
      }
      CONFIG_ADD_DEBUG_MSG( dbg_text , '\n' )
    }

#endif

  }
  else {
    class_name = &DalFactory::instance().get_known_class_name_ref(name);
    CONFIG_ADD_DEBUG_MSG( dbg_text , "  * there is no object with id = \'" << id << "\' found in the class \'" << name << "\' that has no objects in cache\n" )
  }

    // check implementation objects of subclasses

  if(m_conf) {
    config::fmap<config::fset>::const_iterator subclasses = m_conf->subclasses().find(class_name);

    if(subclasses != m_conf->subclasses().end()) {
      for(config::fset::const_iterator k = subclasses->second.begin(); k != subclasses->second.end(); ++k) {
        i = m_impl_objects.find(*k);
        if(i != m_impl_objects.end()) {
          config::map<ConfigObjectImpl *>::const_iterator j = i->second->find(id);

          if(j != i->second->end()) {
            p_number_of_cache_hits++;
  	    CONFIG_ADD_DEBUG_MSG( dbg_text , "  * found the object with id = \'" << id << "\' in class \'" << *k << '\'' )
            ERS_DEBUG(4, dbg_text->str() );
            return j->second;
          }

#ifndef ERS_NO_DEBUG

            // prepare and print out debug message

          else if(ers::debug_level() >= 4) {
            CONFIG_ADD_DEBUG_MSG( dbg_text , "  * there is no object with id = \'" << id << "\' found in the class \'" << *k << "\' that has " << i->second->size() << " objects in cache: " )
            for(j=i->second->begin(); j != i->second->end();++j) {
              if(j != i->second->begin()) { CONFIG_ADD_DEBUG_MSG( dbg_text , ", " ) }
	      CONFIG_ADD_DEBUG_MSG( dbg_text , '\'' << j->first << '\'' )
            }
  	    CONFIG_ADD_DEBUG_MSG( dbg_text , '\n' )
          }

#endif

        }
        else {
          CONFIG_ADD_DEBUG_MSG( dbg_text , "  * there is no object with id = \'" << id << "\' found in the class \'" << *k << "\' that has no objects in cache\n" )
        }

      }
    }

    CONFIG_ADD_DEBUG_MSG( dbg_text , "  * there is no object \'" << id << "\' in class \'" << name << "\' and it's subclasses, returning NULL ..." )
  }
  else {
    CONFIG_ADD_DEBUG_MSG( dbg_text , "  * there is no object \'" << id << "\' in class \'" << name << "\', returning NULL ..." )
  }

  ERS_DEBUG(4, dbg_text->str() );

  return nullptr;
}


void
ConfigurationImpl::put_impl_object(const std::string& name, const std::string& id, ConfigObjectImpl * obj) noexcept
{
  p_number_of_object_read++;

  config::pmap<config::map<ConfigObjectImpl *> * >::iterator i = m_impl_objects.find(&name);

  if(i != m_impl_objects.end()) {
    (*i->second)[id] = obj;
    obj->m_class_name = i->first;
  }
  else {
    config::map<ConfigObjectImpl *> * m = new config::map<ConfigObjectImpl *>();
    obj->m_class_name = &DalFactory::instance().get_known_class_name_ref(name);
    m_impl_objects[obj->m_class_name] = m;
    (*m)[id] = obj;
  }
}

void
ConfigurationImpl::rename_impl_object(const std::string * class_name, const std::string& old_id, const std::string& new_id) noexcept
{
  config::pmap<config::map<ConfigObjectImpl *> *>::iterator i = m_impl_objects.find(class_name);

  if (i != m_impl_objects.end())
    {
      config::map<ConfigObjectImpl *>::iterator j = i->second->find(old_id);

      if (j != i->second->end())
        {
          ConfigObjectImpl*& obj = (*i->second)[new_id];

          if (obj != nullptr)
            {
              obj->m_state = daq::config::Unknown;
              m_tangled_objects.push_back(obj);
            }

          obj = j->second;

          ERS_DEBUG(2, "rename implementation " << (void *)j->second << " of object \'" << old_id << '@' << *class_name << "\' to \'" << new_id << '\'');
          i->second->erase(j);
        }
    }
}

void
ConfigurationImpl::clean() noexcept
{
  for (auto& i : m_impl_objects)
    {
      for (auto& j : *i.second)
        delete j.second;

      delete i.second;
    }

  m_impl_objects.clear();

  for (auto& x : m_tangled_objects)
    delete x;

  m_tangled_objects.clear();
}

std::mutex&
ConfigurationImpl::get_conf_impl_mutex() const
{
  return m_conf->m_impl_mutex;
}

void
ConfigObjectImpl::convert(bool& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(uint8_t& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(int8_t& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(uint16_t& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(int16_t& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(uint32_t& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(int32_t& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(uint64_t& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(int64_t& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(float& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(double& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(std::string& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert(value, obj, attr_name);
}


void
ConfigObjectImpl::convert(std::vector<bool>& /*value*/, const ConfigObject& /*obj*/, const std::string& /*attr_name*/) noexcept
{
  //m_impl->m_conf->convert2(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(std::vector<uint8_t>& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert2(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(std::vector<int8_t>& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert2(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(std::vector<uint16_t>& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert2(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(std::vector<int16_t>& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert2(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(std::vector<uint32_t>& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert2(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(std::vector<int32_t>& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert2(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(std::vector<uint64_t>& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert2(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(std::vector<int64_t>& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert2(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(std::vector<float>& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert2(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(std::vector<double>& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert2(value, obj, attr_name);
}

void
ConfigObjectImpl::convert(std::vector<std::string>& value, const ConfigObject& obj, const std::string& attr_name) noexcept
{
  m_impl->m_conf->convert2(value, obj, attr_name);
}

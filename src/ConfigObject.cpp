#include <vector>
#include <iostream>

#include "config/Configuration.hpp"
#include "config/ConfigurationImpl.hpp"
#include "config/ConfigObject.hpp"
#include "config/ConfigObjectImpl.hpp"
#include "config/Schema.hpp"

ConfigObject::ConfigObject() noexcept :
  m_impl(nullptr)
{
}

ConfigObject::ConfigObject(const ConfigObject& other) noexcept :
  m_impl(other.m_impl)
{
}

ConfigObject::ConfigObject(ConfigObjectImpl *impl) noexcept :
  m_impl(impl)
{
}

ConfigObject::~ConfigObject() noexcept
{
}

ConfigObject&
ConfigObject::operator=(const ConfigObject& other) noexcept
{
  if(this != &other) {
    m_impl = other.m_impl;
  }

  return *this;
}

ConfigObject&
ConfigObject::operator=(ConfigObjectImpl *impl) noexcept
{
  if(m_impl != impl) {
    m_impl = impl;
  }

  return *this;
}

bool
ConfigObject::operator==(const ConfigObject& other) const noexcept
{
  if(this == &other || m_impl == other.m_impl) return true;  // the objects or implementations are the same
  if(!m_impl || !other.m_impl) return false;                 // only one of objects has no implementation
  return ((UID() == other.UID()) && (class_name() == other.class_name()));
}

void
ConfigObject::print_ptr(std::ostream& s) const noexcept
{
  if(m_impl)
    {
      if(m_impl->is_deleted())
        {
          s << "(deleted object " << full_name() << ')';
        }
      else
        {
          s << full_name();
        }
    }
  else
    {
      s << "(null)";
    }
}

Configuration *
ConfigObject::get_configuration() const
{
  return m_impl->m_impl->m_conf;
}

void
ConfigObject::rename(const std::string& new_id)
{
  get_configuration()->rename_object(*this, new_id);
  action_on_object_update(get_configuration(), new_id);
}

void
ConfigObject::action_on_object_update(Configuration * db, const std::string& name)
{
  db->action_on_update(*this, name);
}

std::ostream&
operator<<(std::ostream& s, const ConfigObject * obj)
{
  obj->print_ptr(s);
  return s;
}

std::ostream&
operator<<(std::ostream& s, const ConfigObject & obj)
{
  obj.print_ptr(s);
  return s;
}

inline static void
print_sep(const char sep, std::ostream& s)
{
  if (sep)
    s << sep;
}

template<class T>
  void
  print_val(const T &val, std::ostream &s)
  {
    s << val;
  }

template<>
  void
  print_val<uint8_t>(const uint8_t &val, std::ostream &s)
  {
    s << static_cast<uint16_t>(val);
  }

template<>
  void
  print_val<int8_t>(const int8_t &val, std::ostream &s)
  {
    s << static_cast<int16_t>(val);
  }

template<class T>
  void
  print_value(const ConfigObject& const_obj, const std::string& name, const bool ismv, const char sep, std::ostream& s)
  {
    ConfigObject& obj = const_cast<ConfigObject&>(const_obj);

    try
      {
        if (ismv)
          {
            std::vector<T> value;

            obj.get(name, value);

            print_sep('(', s);

            for (unsigned int i = 0; i < value.size(); ++i)
              {
                if (i != 0)
                  s << ", ";

                print_sep(sep, s);
                print_val<T>(value[i],s);
                print_sep(sep, s);
              }

            print_sep(')', s);
          }
        else
          {
            T value;

            obj.get(name, value);

            print_sep(sep, s);
            print_val<T>(value,s);
            print_sep(sep, s);
          }
      }
    catch (ers::Issue & ex)
      {
        s << "[bad_object] (could not get value of \'" << name << "\' of object \'" << &obj << "\': " << ex << ')';
      }
  }

// workaround to avoid annoying warning about comparing this with nullptr
inline bool
is_null_obj(const ConfigObject * o)
{
  return (o == nullptr || o->is_null());
}

void
ConfigObject::print_ref(std::ostream& s, ::Configuration& config, const std::string& prefix, bool show_contained_in) const noexcept
{
  static bool expand_aggregation = (getenv("TDAQ_CONFIG_PRINT_EXPAND_AGGREGATIONS")); // FIXME tdaq-09-05-00 => add new parameter to config and add fuse

  // check if it is not a reference to 0
  if (is_null_obj(this))
    {
      s << prefix << "(null)";
      return;
    }

  // print out object-id and class-name
  s
    << prefix << "Object:\n"
    << prefix << "  id: \'" << UID() << "\', class name: \'" << class_name() << "\'\n";

  if (show_contained_in)
    s << prefix << "  contained in: \'" << contained_in() << "\'\n";

  try
    {
      const daq::config::class_t& cd(config.get_class_info(class_name()));

      // print attributes
      for (const auto& i : cd.p_attributes)
        {
          const std::string& aname(i.p_name);    // attribute name
          const bool ismv(i.p_is_multi_value);   // attribute is multi-value

          s << prefix << "  " << aname << ": ";

          switch (i.p_type)
            {
              case daq::config::string_type :
              case daq::config::enum_type :
              case daq::config::date_type :
              case daq::config::time_type :
              case daq::config::class_type :
                                             print_value<std::string>(*this, aname, ismv, '\"', s); break;
              case daq::config::bool_type:   print_value<bool>(*this, aname, ismv, 0, s);           break;
              case daq::config::u8_type:     print_value<uint8_t>(*this, aname, ismv, 0, s);        break;
              case daq::config::s8_type:     print_value<int8_t>(*this, aname, ismv, 0, s);         break;
              case daq::config::u16_type:    print_value<uint16_t>(*this, aname, ismv, 0, s);       break;
              case daq::config::s16_type:    print_value<int16_t>(*this, aname, ismv, 0, s);        break;
              case daq::config::u32_type:    print_value<uint32_t>(*this, aname, ismv, 0, s);       break;
              case daq::config::s32_type:    print_value<int32_t>(*this, aname, ismv, 0, s);        break;
              case daq::config::u64_type:    print_value<uint64_t>(*this, aname, ismv, 0, s);       break;
              case daq::config::s64_type:    print_value<int64_t>(*this, aname, ismv, 0, s);        break;
              case daq::config::float_type:  print_value<float>(*this, aname, ismv, 0, s);          break;
              case daq::config::double_type: print_value<double>(*this, aname, ismv, 0, s);         break;
              default:                       s << "*** bad type ***";
            }

          s << std::endl;
        }

      // print relationships
      for (const auto& i : cd.p_relationships)
        {
          s << prefix << "  " << i.p_name << ':';
          if (expand_aggregation == false || i.p_is_aggregation == false)
            {
              s << ' ';
              print_value<ConfigObject>(*this, i.p_name, (i.p_cardinality == daq::config::zero_or_many) || (i.p_cardinality == daq::config::one_or_many), '\"', s);
              s << std::endl;
            }
          else
            {
              s << std::endl;
              std::string prefix2(prefix + "    ");
              ConfigObject& obj = const_cast<ConfigObject&>(*this);
              if ((i.p_cardinality == daq::config::zero_or_many) || (i.p_cardinality == daq::config::one_or_many))
                {
                  std::vector<ConfigObject> value;
                  obj.get(i.p_name, value);
                  if (value.empty())
                    s << prefix2 << "(null)\n";
                  else
                    for (const auto& x : value)
                      x.print_ref(s, config, prefix2, show_contained_in);
                }
              else
                {
                  ConfigObject value;
                  obj.get(i.p_name, value);
                  if (value.is_null())
                    s << prefix2 << "(null)\n";
                  else
                    value.print_ref(s, config, prefix2, show_contained_in);
                }
            }
        }
    }
  catch (daq::config::Exception& ex)
    {
      s << "cannot get schema description: caught daq::config::Exception exception" << std::endl;
      std::cerr << "ERROR: " << ex << std::endl;
    }

}

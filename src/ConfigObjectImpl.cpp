#include "config/ConfigObject.h"
#include "config/ConfigObjectImpl.h"

class ConfigObjectDefault : public ConfigObjectImpl {

  private:
  
    /// \throw daq::config::Generic
    static void bad();
    static const std::string s_invalid;

  public:

    ConfigObjectDefault() noexcept : ConfigObjectImpl(0, s_invalid) {}
    virtual ~ConfigObjectDefault() noexcept {}


  public:

    virtual const std::string contained_in() const { bad(); return s_invalid; }

    virtual void get(const std::string& /*attribute*/,   bool&           /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   uint8_t&        /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   int8_t&         /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   uint16_t&       /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   int16_t&        /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   uint32_t&       /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   int32_t&        /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   uint64_t&       /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   int64_t&        /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   float&          /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   double&         /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   std::string&    /*value*/) { bad(); }
    virtual void get(const std::string& /*association*/, ConfigObject&   /*value*/) { bad(); }

    virtual void get(const std::string& /*attribute*/,   std::vector<bool>&           /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   std::vector<uint8_t>&        /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   std::vector<int8_t>&         /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   std::vector<uint16_t>&       /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   std::vector<int16_t>&        /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   std::vector<uint32_t>&       /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   std::vector<int32_t>&        /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   std::vector<uint64_t>&       /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   std::vector<int64_t>&        /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   std::vector<float>&          /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   std::vector<double>&         /*value*/) { bad(); }
    virtual void get(const std::string& /*attribute*/,   std::vector<std::string>&    /*value*/) { bad(); }
    virtual void get(const std::string& /*association*/, std::vector<ConfigObject>&   /*value*/) { bad(); }

    virtual bool rel(const std::string& /*association*/, std::vector<ConfigObject>& /*value*/) { bad(); return false; }
    virtual void referenced_by(std::vector<ConfigObject>& /*value*/, const std::string& /*association*/, bool /*check_composite_only*/, unsigned long /*rlevel*/, const std::vector<std::string> * /*rclasses*/) const { bad(); }

    virtual void set(const std::string& /*attribute*/, bool               /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/, uint8_t            /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/, int8_t             /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/, uint16_t           /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/, int16_t            /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/, uint32_t           /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/, int32_t            /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/, uint64_t           /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/, int64_t            /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/, float              /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/, double             /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/, const std::string& /*value*/) { bad(); }

    virtual void set_enum(const std::string& /*attribute*/, const std::string& /*value*/) { bad(); }
    virtual void set_date(const std::string& /*attribute*/, const std::string& /*value*/) { bad(); }
    virtual void set_time(const std::string& /*attribute*/, const std::string& /*value*/) { bad(); }

    virtual void set_class(const std::string& /*attribute*/, const std::string& /*value*/) { bad(); }

    virtual void set(const std::string& /*attribute*/,   const std::vector<bool>&           /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/,   const std::vector<uint8_t>&        /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/,   const std::vector<int8_t>&         /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/,   const std::vector<uint16_t>&       /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/,   const std::vector<int16_t>&        /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/,   const std::vector<uint32_t>&       /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/,   const std::vector<int32_t>&        /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/,   const std::vector<uint64_t>&       /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/,   const std::vector<int64_t>&        /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/,   const std::vector<float>&          /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/,   const std::vector<double>&         /*value*/) { bad(); }
    virtual void set(const std::string& /*attribute*/,   const std::vector<std::string>&    /*value*/) { bad(); }

    virtual void set_enum(const std::string& /*attribute*/, const std::vector<std::string>& /*value*/) { bad(); }
    virtual void set_date(const std::string& /*attribute*/, const std::vector<std::string>& /*value*/) { bad(); }
    virtual void set_time(const std::string& /*attribute*/, const std::vector<std::string>& /*value*/) { bad(); }

    virtual void set_class(const std::string& /*attribute*/, const std::vector<std::string>& /*value*/) { bad(); }

    virtual void set(const std::string& /*association*/, const ConfigObject*                     /*value*/, bool) { bad(); }
    virtual void set(const std::string& /*association*/, const std::vector<const ConfigObject*>& /*value*/, bool) { bad(); }

    virtual void move(const std::string& /*at*/) { bad(); }
    virtual void rename(const std::string& /*new_id*/) { bad(); }

    virtual void reset() { bad(); }

};

ConfigObjectImpl::ConfigObjectImpl(ConfigurationImpl * impl, const std::string& id, daq::config::ObjectState state) noexcept : m_impl (impl), m_state(state), m_id(id), m_class_name(nullptr)
{
}

ConfigObjectImpl::~ConfigObjectImpl() noexcept
{
}

ConfigObjectImpl *ConfigObjectImpl::default_impl() noexcept
{
  return new ConfigObjectDefault();
}

const std::string ConfigObjectDefault::s_invalid = "*INVALID*";

void ConfigObjectDefault::bad()
{
  throw daq::config::Generic( ERS_HERE, "access *INVALID* object");
}

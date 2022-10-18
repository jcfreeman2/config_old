# config

## tdaq-09-03-00

### Java exceptions become checked

Now, the java config exceptions are derived from **ers.Issue**. This makes them checked. Most methods of config and generated DAL classes might throw an exception, that either must be caught in try / catch block or forwarded. The **config.ConfigException** base class can be used to simplify exceptions handling.

```
try
  {
    config.Configuration db = new config.Configuration("");
    dal.Partition p = dal.Algorithms.get_partition(db, "test");
  }
catch(final config.ConfigException ex)
  {
    ers.Logger.error(ex);
  }
```

Any jar using config or generated DAL must include **Jers/ers.jar** in CMakeLists.txt.

### Export configuration to Boost ptree

There are two new methods in the Configuration class to export configuration data and schema to the Boost.PropertyTree:

```
void export_schema(boost::property_tree::ptree& tree, ...);
void export_data(boost::property_tree::ptree& tree, ...);
```

The classes, objects and data files can be selected using regular expressions.

The schema ptree follows this structure:

```
{
    "class-name" {
        "abstract" : <value>
        "description" : <value>
        "superclasses" [
            <value>,
            ...
        ]
        "attributes" {
          "attribute-name" {
            "type" : <value>
            "range" : <value>
            "format" : <value>
            "is-not-null" : <value>
            "is-multi-value" : <value>
            "default-value" : <value>
            "description" : <value>
          },
          ...
        }
        "relationships" {
          "relationship-name" {
            "type" : <value>
            "cardinality" : <value>
            "is-aggregation" : <value>
            "description" : <value>
          },
          ...
        }
    }
}
```

The "superclasses", "attributes" and "relationships" nodes are only added, when non-empty. Similarly, the "description", "range", "format", "is-not-null", "is-multi-value", "default-value" and "is-aggregation" elements are only added, when meaningful.

The data ptree follows this structure:

```
{
    "class-name" {
        "object-id" {
            "attribute-name" : <value>
            "multivalue-attribute-name" [
              <value>,
              ...
            ]
            "relationship-name" : <value>
            "multivalue-relationship-name" [
              <value>,
              ...
            ]
        },
        ...
    },
    ...
}
```

A relationship value is stored in oks format: "object-id@class-name". The multivalue attributes and relationships are stored as arrays (can be empty).

There are two new utilities to export schema and data into json, xml and ptree-info formats using above methods:
* config_export_schema
* config_export_data

### Rename object in python binding

The python binding supports now renaming an OKS object.

```python
import config

db = config.Configuration('oksconfig:myfile.data.xml')
hltsv = db.get_obj('HLTSVApplication', 'HLTSV')
hltsv.rename('HLTSV-new-name')
db.commit('Changed name of HLT supervisor application')
```

On the DAL level:

```python
import config

db = config.Configuration('oksconfig:myfile.data.xml')
hltsv = db.get_dal('HLTSVApplication', 'HLTSV')
hltsv.rename('HLTSV-new-name')
db.update_dal(hltsv)
db.commit('Changed name of the HLT supervisor application')
```


## tdaq-09-01-00

### New way to access config versions

Jira: [ADTCC-228](https://its.cern.ch/jira/browse/ADTCC-228)

Using oks git repository one can access details of the available config versions. The details of version are described by methods of new c++ Version class:
```
const std::string& get_id() const noexcept;
const std::string& get_user() const noexcept;
const std::time_t get_timestamp() const noexcept;
const std::string& get_comment() const noexcept;
const std::vector<std::string>& get_files() const noexcept;
```
and similar java ones:
```
String get_id();
String get_user();
Instant get_timestamp();
String get_comment();
String[] get_files();
```

To obtain new versions created on remote origin after currently used version, or externally modified files, use new get_changes() method:
```
std::vector<daq::config::Version> get_changes(); // c++
Version[] get_changes(); // java
```

To access historical versions use new get_versions() method:
```
enum QueryType { query_by_date, query_by_id, query_by_tag };

std::vector<dVersion> get_versions(const std::string& since, const std::string& until, QueryType type, bool skip_irrelevant); // c++
Version[] get_versions(String since, String until, Version.QueryType type, boolean skip_irrelevant); // java
```

The config_dump application has new command line option to print information about archived versions:
```
-v | --versions  rint details of versions providing 4 parameters all|skip date|id|tag since until
```

For example:
```
$ config_dump -d oksconfig:daq/schema/core.schema.xml -v all date '2020-05-19' '2020-05-20'
```

### create() database method

Jira: [ADTCC-240](https://its.cern.ch/jira/browse/ADTCC-240)

Remove the server_name argument in the Configuration create() method. In case of rdbconfig implementation the user has to connect with rdb writer before calling create() method.


## tdaq-08-03-01

### Remove obsolete get_class_info() method using MetaDataType enum
Jira: [ADTCC-184](https://its.cern.ch/jira/browse/ADTCC-184)

Use well structured method instead:
```
const daq::config::class_t& get_class_info(const std::string& class_name, bool direct_only = false);
```

### Dynamic get() method
Jira: [ADTCC-177](https://its.cern.ch/jira/browse/ADTCC-177)

The new method allows to read values of relationships and execute parameter-less algorithms on abstract DalObjects
```
virtual std::vector<const DalObject *> DalObject::get(const std::string& name, bool upcast_unregistered = true) const = 0;
```
where the name parameter is a name of the relationship or algorithm. The returned DAL objects can be casted to DAL classes using generated cast() method.

The implementations of the DAL objects, relationships and algorithms not need to be known at compilation time, however appropriate generated DAL libraries have to be linked with user code. In case if name is not known to DAL, run-time exception will be thrown.

This method allows to write schema-independent code on DAL layer.



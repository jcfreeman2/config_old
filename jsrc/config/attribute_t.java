package config;

/**
 * Describe properties of an attribute.
 */

public final class attribute_t
  {

    /**
     * The supported attribute types.
     */

    public enum type_t {

      /** the boolean type */
      bool_type,

      /** the 8-bits signed integer type */
      s8_type,

      /** the 8-bits unsigned integer type */
      u8_type,

      /** the 16-bits signed integer type */
      s16_type,

      /** the 16-bits unsigned integer type */
      u16_type,

      /** the 32-bits signed integer type */
      s32_type,

      /** the 32-bits unsigned integer type */
      u32_type,

      /** the 64-bits signed integer type */
      s64_type,

      /** the 64-bits unsigned integer type */
      u64_type,

      /** the float type */
      float_type,

      /** the double type */
      double_type,

      /** the date type */
      date_type,

      /** the time type */
      time_type,

      /** the string type */
      string_type,

      /** the enumeration type */
      enum_type,

      /** the class reference type */
      class_type

    };

    public enum int_format_t {

      /** use octal numeration for attribute value */
      oct_int_format,

      /** use decimal numeration for attribute value */
      dec_int_format,

      /** use hexadecimal numeration for attribute value */
      hex_int_format,

      /** not applicable, the attribute type is not integer */
      na_int_format

    };

    private String p_name;
    private type_t p_type;
    private String p_range;
    private int_format_t p_int_format;
    private boolean p_is_not_null;
    private boolean p_is_multi_value;
    private String p_default_value;
    private String p_description;

    public attribute_t(String name, type_t type, String range, int_format_t int_format, boolean is_not_null, boolean is_multi_value,
        String default_value, String description)
      {
        p_name = name;
        p_type = type;
        p_range = range;
        p_int_format = int_format;
        p_is_not_null = is_not_null;
        p_is_multi_value = is_multi_value;
        p_default_value = default_value;
        p_description = description;
      }

    /**
     * Get the attribute name
     */
    public String get_name()
      {
        return p_name;
      }

    /**
     * Get the attribute type
     */
    public type_t get_type()
      {
        return p_type;
      }

    /**
     * Get the attribute range in UML syntax (e.g.: "A,B,C..D,*..F,G..*" corresponds a
     * value can be A, B, greater or equal to C and less or equal to D, less or
     * equal to F, greater or equal to G)
     */
    public String get_range()
      {
        return p_range;
      }

    /**
     * Get the representation format for integer values
     */
    public int_format_t get_int_format()
      {
        return p_int_format;
      }

    /**
     * Return true, if the value of attribute cannot be null
     */
    public boolean get_is_not_null()
      {
        return p_is_not_null;
      }

    /**
     * Return true, if the value of attribute is a list of primitive values
     * (e.g. list of strings, list of integer numbers, etc.)
     */
    public boolean get_is_multi_value()
      {
        return p_is_multi_value;
      }

    /**
     * Get the default value of attribute
     */
    public String get_default_value()
      {
        return p_default_value;
      }

    /**
     * Get the description of attribute
     */
    public String get_description()
    {
      return p_description;
    }

    /**
     * Print attribute info to standard output
     * 
     * @param prefix
     *          string prefix (shift used for nested types)
     */
    public void print(String prefix)
      {
        System.out.println(prefix + "attribute \'" + get_name() + '\'');
        System.out.println(prefix + "  type: \'" + type2str(get_type()) + '\'');
        System.out.println(prefix + "  range: \'" + get_range() + '\'');

        if (get_int_format() != int_format_t.na_int_format)
          {
            System.out.println(prefix + "  integer format: \'" + format2str(get_int_format()) + '\'');
          }

        System.out.println(prefix + "  is not null: \'" + get_is_not_null() + '\'');
        System.out.println(prefix + "  is multi-value: \'" + get_is_multi_value() + '\'');
        System.out.println(prefix + "  default value: \'" + get_default_value() + '\'');
        System.out.println(prefix + "  description: \'" + get_description() + '\'');
      }
    
    
    private String type2str(type_t type)
      {
        switch (type)
          {
          case bool_type:
            return "boolean";
          case s8_type:
            return "8-bits signed integer";
          case u8_type:
            return "8-bits unsigned integer";
          case s16_type:
            return "16-bits signed integer";
          case u16_type:
            return "16-bits unsigned integer";
          case s32_type:
            return "32-bits signed integer";
          case u32_type:
            return "32-bits unsigned integer";
          case s64_type:
            return "64-bits signed integer";
          case u64_type:
            return "64-bits unsigned integer";
          case float_type:
            return "float";
          case double_type:
            return "double";
          case date_type:
            return "date";
          case time_type:
            return "time";
          case string_type:
            return "string";
          case enum_type:
            return "enumeration";
          case class_type:
            return "class reference";
          default:
            return "unknown";
          }
      }

    private String format2str(int_format_t type)
      {
        switch (type)
          {
          case oct_int_format:
            return "octal";
          case dec_int_format:
            return "decimal";
          case hex_int_format:
            return "hexadecimal";
          default:
            return "not applicable";
          }
      }

  }
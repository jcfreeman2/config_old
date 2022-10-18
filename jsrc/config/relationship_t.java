package config;


/**
 * Describe properties of a relationship.
 */

public final class relationship_t
  {
    public enum cardinality_t {

      /** relationship references zero or one object */
      zero_or_one,

      /** relationship references zero or many objects */
      zero_or_many,

      /** relationship always references one object */
      only_one,

      /** relationship references one or many objects */
      one_or_many

    };

    private String p_name;
    private String p_type;
    private cardinality_t p_cardinality;
    private boolean p_is_aggregation;
    private String p_description;

    public relationship_t(String name, String type, boolean can_be_null, boolean is_multi_value, boolean is_aggregation, String description)
      {
        p_name = name;
        p_type = type;
        p_cardinality = (
          (can_be_null && !is_multi_value) ? cardinality_t.zero_or_one  :
          (can_be_null && is_multi_value)  ? cardinality_t.zero_or_many :
          (!can_be_null && is_multi_value) ? cardinality_t.one_or_many  :
          cardinality_t.only_one
        );
        p_is_aggregation = is_aggregation;
        p_description = description;
      }

    /**
     * Get the relationship name
     */
    public String get_name()
      {
        return p_name;
      }

    /**
     * Get the relationship class type
     */
    public String get_type()
      {
        return p_type;
      }

    /**
     * Get the relationship cardinality
     */
    public cardinality_t get_cardinality()
      {
        return p_cardinality;
      }

    /**
     * Return true, if the relationship is an aggregation (composite); otherwise
     * the relationship is simple (weak)
     */
    public boolean get_is_aggregation()
      {
        return p_is_aggregation;
      }

    /**
     * Get the description of relationship
     */
    public String get_description()
      {
        return p_description;
      }
    
    /**
     * Print relationship info to standard output
     * 
     * @param prefix
     *          string prefix (shift used for nested types)
     */
    public void print(String prefix)
      {
        System.out.println(prefix + "relationship \'" + get_name() + '\'');
        System.out.println(prefix + "  class type: \'" + get_type() + '\'');
        System.out.println(prefix + "  cardinality: \'" + card2str(get_cardinality()) + '\'');
        System.out.println(prefix + "  is aggregation: \'" + get_is_aggregation() + '\'');
        System.out.println(prefix + "  description: \'" + get_description() + '\'');
      }
    
    private String card2str(cardinality_t cardinality)
      {
        switch (cardinality)
          {
          case zero_or_one:
            return "zero or one";
          case zero_or_many:
            return "zero or many";
          case only_one:
            return "one";
          case one_or_many:
            return "one or many";
          default:
            return "unknown";
          }

      }
  }

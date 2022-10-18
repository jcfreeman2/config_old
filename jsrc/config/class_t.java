package config;

/**
 * Describe properties of a class.
 */

public final class class_t
  {
    private String p_name;
    private String p_description;
    private boolean p_abstract;
    private String[] p_superclasses;
    private String[] p_subclasses;
    private attribute_t[] p_attributes;
    private relationship_t[] p_relationships;

    /**
     * Get the class name
     */
    public String get_name() { return p_name; }

    /**
     * Get the description text of class
     */
    public String get_description() { return p_description; }

    /**
     * Return true, if the class is abstract and has no objects
     */
    public boolean get_abstract() { return p_abstract; }

    /**
     * Get the names of direct superclasses
     */
    public String[] get_superclasses() { return p_superclasses; }

    /**
     * Get the names of direct subclasses
     */
    public String[] get_subclasses() { return p_subclasses; }

    /**
     * Get the all attributes of the class
     */
    public attribute_t[] get_attributes() { return p_attributes; }

    /**
     * Get the all relationships of the class
     */
    public relationship_t[] get_relationships() { return p_relationships; }
    
    
      /** Create class description */

    public class_t(String name, String description, boolean is_abstract, String[] superclasses, String[] subclasses, attribute_t[] attributes, relationship_t[] relationships)
      {
        p_name = name;
        p_description = description;
        p_abstract = is_abstract;
        p_superclasses = superclasses;
        p_subclasses = subclasses;
        p_attributes = attributes;
        p_relationships = relationships;
      }

    public void print(String prefix)
      {
        System.out.println(prefix + "class \'" + get_name() + '\'');
        System.out.println(prefix + "  is abstract: \'" + get_abstract() + '\'');
        System.out.println(prefix + "  description: \'" + get_description() + '\'');
        
        if(p_superclasses != null)
          {
            System.out.println(prefix + "  " + p_superclasses.length + " superclass(es):");

            for(String x : p_superclasses)
              {
                System.out.println(prefix + "   \'" + x + '\'');
              }
          }
        else
          {
            System.out.println(prefix + "  there are no superclasses");
          }
        
        if(p_subclasses != null)
          {
            System.out.println(prefix + "  " + p_subclasses.length + " subclass(es):");

            for(String x : p_subclasses)
              {
                System.out.println(prefix + "   \'" + x + '\'');
              }
          }
        else
          {
            System.out.println(prefix + "  there are no subclasses");
          }
        

        final String new_prefix = prefix + "    ";

        if(p_attributes != null)
          {
            System.out.println(prefix + "  " + p_attributes.length + " attribute(s):");

            for(attribute_t x : p_attributes)
              {
                x.print(new_prefix);
              }
          }
        else
          {
            System.out.println(prefix + "  there are no attributes");
          }

        if(p_relationships != null)
          {
            System.out.println(prefix + "  " + p_relationships.length + " relationship(s):");

            for(relationship_t x : p_relationships)
              {
                x.print(new_prefix);
              }
          }
        else
          {
            System.out.println(prefix + "  there are no relationships");
          }
      }

  }

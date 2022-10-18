package config;

import java.util.Vector;

  /**
   *  The <b>Change</b> class is used to describe subset of the database changes happen
   *  after a database modification which satisfies to the subscription criteria.
   *  The changes description includes:
   *  <ul>
   *   <li><b>the class name</b> accessible via {@link #get_class_name()} method
   *   <li>the array of <b>the created objects identities</b> accessible via {@link #get_created_objects()} method
   *   <li>the array of <b>the changed objects identities</b> accessible via {@link #get_changed_objects()} method
   *   <li>the array of <b>the deleted objects identities</b> accessible via {@link #get_changed_objects()} method
   *  </ul>
   *  For debug purposes it is possible to use {@link #print(String) print} method for a change object
   *  and another {@link #print(Change[], String) print} method for an array of change objects.
   *
   *  @author  http://consult.cern.ch/xwho/people/432778
   *  @since   online release 00-21-00
   */

public class Change {
  private String p_name;
  private String[] p_created;
  private String[] p_changed;
  private String[] p_deleted;

    /** Returns name of the modified class. */

  public String get_class_name() {
    return p_name;
  }

    /** Returns array of identities of the created objects. */

  public String[] get_created_objects() {
    return p_created;
  }

    /** Returns array of identities of the changed objects. */

  public String[] get_changed_objects() {
    return p_changed;
  }

    /** Returns array of identities of the deleted objects. */

  public String[] get_deleted_objects() {
    return p_deleted;
  }

    /**
     *  Constructor used by objects of the database implementation classes.
     *  @param s        the class name
     *  @param created  array of the created objects identities
     *  @param changed  array of the changed objects identities
     *  @param deleted  array of the deleted objects identities
     */

  public Change(String s, String[] created, String[] changed, String[] deleted ) {
    p_name = s;
    p_created = created;
    p_changed = changed;
    p_deleted = deleted;
  }

   /**
    *  Constructor used by the Configuration object.
    *  @param s        the class name
    *  @param created  vector of the created objects identities
    *  @param changed  vector of the changed objects identities
    *  @param deleted  vector of the deleted objects identities
   */

  protected Change(String s, Vector<String> created, Vector<String> changed, Vector<String> deleted ) {
    p_name = s;
    p_created = vector2array(created);
    p_changed = vector2array(changed);
    p_deleted = vector2array(deleted);
  }


   /**
    *  Method to print changes description.
    *  To be used for debug purposes.
    *  @param dx string for left margin (for a formatted output)
    */

  public void print(String dx) {
    System.out.println( dx + "class name: \"" + p_name + "\"" );

    Change.print(dx, p_created, "created");
    Change.print(dx, p_changed, "changed");
    Change.print(dx, p_deleted, "deleted");
  }

   /**
    *  Method to print an array of changes.
    *  To be used for debug purposes.
    *  @param changes  array of objects describing database changes
    *  @param dx       string for left margin (for a formatted output)
    */

  static public void print(Change[] changes, String dx) {
    if(changes.length > 0) {
      System.out.println( dx + "changes in " + changes.length + " classes:");
      for(int i = 0; i < changes.length; ++i) {
        changes[i].print(dx + "  ");
      }
    }
    else {
      System.out.println( dx + "(null changes)" );
    }
  }

  static private void print(String dx, String[] value, String name) {
    if(value == null || value.length == 0) {
      System.out.println( dx + " * no " + name + " objects" );
    }
    else {
      System.out.println( dx + " * " + value.length + " " + name + " objects:" );
      for(int i = 0; i < value.length; ++i) {
        System.out.println( dx + "   - " + value[i]);
      }
    }
  }

  static private String[] vector2array(Vector<String> v) {
    if(v != null) {
      String[] s = new String[v.size()];
      for(int i = 0; i < v.size(); ++i) {
        s[i] = v.get(i);
      }
      
      return s;
    }
    else {
      return null;
    }
  }

}

package config;

import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.LinkedList;
import java.util.List;
import java.util.TreeSet;

class ConfigObjectComparator implements Comparator<config.ConfigObject>
{
    public int compare(config.ConfigObject o1, config.ConfigObject o2)
    {
       return o1.UID().compareTo(o2.UID());
   }
}

public class ConfigDump {

    static private void usage()
      {
        System.out.println("Usage: java ... Dump -d dbspec [-c | -C [class_name]] [-o | -O [object_id]]");
        System.out.println();
        System.out.println("Options/Arguments:");
        System.out.println("  -d | --database dbspec            database specification in format plugin-name:parameters");
        System.out.println("  -c | --class-direct-info [name]   print direct properties of all classes, or given class if name is provided");
        System.out.println("  -C | --class-all-info [name]      same as -c, but print all properties of class (all attributes, all superclasses, etc.)");
        System.out.println("  -o | --list-objects               list objects of class");
        System.out.println("  -r | --print-referenced-by        print objects referencing given object (only with -o option)");
        System.out.println("  -O | --dump-objects [id]          dump all objects of class or details of given object, if id is provided (-c is required)");
        System.out.println();
        System.out.println("Description:");
        System.out.println("  The utility dumps class and objects descriptions using abstract java config API.");
        System.out.println("  When no -c or -o options are provided, utility lists all classes.");
      }
    
    static private void print_referenced_by(config.ConfigObject obj, String prefix) throws NotFoundException, SystemException
      {
        LinkedList<config.ConfigObject> values = new LinkedList<config.ConfigObject>();
        obj.referenced_by(values, "*", false);
        if (values.size() == 0)
          {
            System.out.println(prefix + "is not referenced by others objects");
          }
        else
          {
            System.out.println(prefix + "is referenced by " + values.size() + " object" + (values.size() == 1 ? "" : "s") + ':');
            for (config.ConfigObject o : values)
              {
                System.out.print(prefix + " * ");
                o.print();
              }
          }
      }

    public static void main(String args[])
      {

        final String any = "*";

        String db_name = null;
        String class_name = null;
        String object_id = null;

        boolean direct_info = false;
        boolean objects_details = false;
        boolean referenced_by = false;


        for (int i = 0; i < args.length; i++)
          {
            if (args[i].equals("-d") || args[i].equals("--database"))
              {
                db_name = args[++i];
              }
            else if (args[i].equals("-c") || args[i].equals("--class-direct-info"))
              {
                direct_info = true;
                if ((i + 1) < args.length && args[i + 1].charAt(0) != '-')
                  {
                    class_name = args[++i];
                  }
                else
                  {
                    class_name = any;
                  }
              }
            else if (args[i].equals("-C") || args[i].equals("--class-all-info"))
              {
                direct_info = false;
                if ((i + 1) < args.length && args[i + 1].charAt(0) != '-')
                  {
                    class_name = args[++i];
                  }
                else
                  {
                    class_name = any;
                  }
              }
            else if (args[i].equals("-o") || args[i].equals("--list-objects"))
              {
                objects_details = false;
                object_id = any;
              }
            else if (args[i].equals("-r") || args[i].equals("--print-referenced-by"))
              {
                referenced_by = true;
              }
            else if (args[i].equals("-O") || args[i].equals("--dump-objects"))
              {
                objects_details = true;
                if ((i + 1) < args.length && args[i + 1].charAt(0) != '-')
                  {
                    object_id = args[++i];
                  }
                else
                  {
                    object_id = any;
                  }
              }
            else if (args[i].equals("-h") || args[i].equals("--help"))
              {
                usage();
                System.exit(0);
              }
            else
              {
                System.err.println("ERROR [Dump.main()]: unexpected parameter \'" + args[i] + "\'\n");
                usage();
                System.exit(1);
              }
          }
        
        if(db_name == null)
          {
            System.err.println("FATAL [Dump.main()]: no database name given");
            System.exit(1);
          }
        
        if(class_name == null && object_id != null && object_id != any)
          {
            System.err.println("FATAL [Dump.main()]: object id is set, but no class name given (use -c option)");
            System.exit(1);
          }

        try
          {
            config.Configuration db = new config.Configuration(db_name);

            TreeSet<String> classes = new TreeSet<String>();

            if (class_name == null || class_name == any)
              {
                for (String c : db.superclasses().keySet())
                  {
                    classes.add(c);
                  }
              }
            else if (class_name != null)
              {
                classes.add(class_name);
              }


            if (object_id == null)
              {
                // if there are no options, just list classes
                if (class_name == null)
                  {
                    System.out.println("The database schema has " + classes.size() + " class(es):");

                    for (String c : classes)
                      {
                        System.out.println(" - \'" + c + "\'");
                      }
                  }

                // or print details of classes
                else
                  {
                    if(class_name == any)
                      {
                        System.out.println("The database schema has " + classes.size() + " class(es):");
                      }

                    for (String c : classes)
                      {
                        db.get_class_info(c, direct_info).print("  ");
                      }
                  }

                System.exit(0);
              }
     
            String prefix  = "";
            String prefix2 = "  ";
            String prefix3 = "    ";

            // list or print all objects of class(es)
            if(object_id == any)
              {
                if(class_name == null || class_name == any)
                  {
                    System.out.println("The database schema has " + classes.size() + " class(es):");
                    prefix  = "  ";
                    prefix2 = "    ";
                    prefix3 = "      ";
                  }

                for(String c : classes)
                  {
                    config.ConfigObject[] objs = db.get_objects(c, new config.Query());
                    
                    if(objs == null || objs.length == 0)
                      {
                        System.out.println(prefix + "The class \'" + c + "\' has no objects");
                      }
                    else
                      {
                        System.out.println(prefix + "The class \'" + c + "\' has " + objs.length + " object(s) including sub-classes:");

                        List<config.ConfigObject> sorted_objs = Arrays.asList(objs);
                        Collections.sort(sorted_objs, new ConfigObjectComparator());

                        for(config.ConfigObject o : sorted_objs)
                          {
                            if(o.class_name().compareTo(c) == 0)
                              {
                                if(objects_details == true)
                                  {
                                    o.print_ref(prefix2);
                                  }
                                else
                                  {
                                    System.out.println(prefix + " - \'" + o.UID() + '\'');
                                  }
                                
                                if(referenced_by)
                                  {
                                    print_referenced_by(o, prefix3);
                                  }
                              }
                            else
                              {
                                System.out.println(prefix + " - skip \'" + o.UID() + "\' (database class name = \'" + o.class_name() + "\')");
                              }
                          }
                      }
                  }
                
                System.exit(0);
              }
            else
              {
                config.ConfigObject obj = db.get_object(class_name, object_id);
                obj.print_ref(prefix2);

                if(referenced_by) {
                  print_referenced_by(obj, prefix2);
                }
              }

          }
        catch (final config.ConfigException ex)
          {
        	ers.Logger.error(ex);
            System.exit(1);
          }
      }
  }

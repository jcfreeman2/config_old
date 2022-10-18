package config;

import java.util.Hashtable;
import java.util.TreeMap;
import java.util.TreeSet;

  /**
   *  The interface defines based methods to be supported by a database implementation.
   *
   *  @author  http://consult.cern.ch/xwho/people/432778
   *  @since   online release 00-21-00
   */

public interface ConfigurationImpl
{

    /**
     *  Opens database with given name.
     *  Throws exception if failed.
     *  @param db_name database name
     */

  void open_db(String db_name) throws SystemException;

    /**
     *  Closes database.
     *  Throws exception if failed.
     */

  void close_db() throws SystemException;

    /** Returns database load status. Returns <b>true</b> if a database is loaded. */

  boolean loaded();
  
  void set_configuration(Configuration db);

  Configuration get_configuration();


    /** Return hierarchy of superclasses (from database schema) */

  TreeMap<String,TreeSet<String>> get_superclasses() throws config.SystemException;


    /** Creates new database file.
     *
     *  If server with such name can not be found or connected, the {@link SystemException} is thrown.
     *  If file already exists, the {@link NotAllowedException} is thrown.
     *  If there are no permissions to create new file, the {@link AlreadyExistsException} is thrown.
     *
     *  @param server_name   server to be used to create new database file
     *  @param db_name       name of new database file (must be an absolute path to non-existing file)
     *  @param includes      optional list of others database files to be included
     */

  void create(String server_name, String db_name, String[] includes) throws SystemException, NotAllowedException, AlreadyExistsException;


	  /** Creates new database file.
	  *
	  *  In case of rdb implementation, the server has to be connected using open_db().
	  *  If file already exists, the {@link NotAllowedException} is thrown.
	  *  If there are no permissions to create new file, the {@link AlreadyExistsException} is thrown.
	  *
	  *  @param db_name       name of new database file (must be an absolute path to non-existing file)
	  *  @param includes      optional list of others database files to be included
	  */
	
	void create(String db_name, String[] includes) throws SystemException, NotAllowedException, AlreadyExistsException;
  
  
    /**
     *  Add include file to existing database.
     *
     *  If database <code>db_name</code> is not loaded, the {@link NotFoundException} is thrown.
     *  If there are no permissions to modify the database, the {@link NotFoundException} is thrown.
     *
     *  @param db_name       name of database file to be included
     *  @param include       file to be included
     */

  void add_include(String db_name, String include) throws SystemException, NotFoundException, NotAllowedException;

    /**
     *  Remove include file from existing database.
     *
     *  If database <code>db_name</code> is not loaded or there is no such include, the {@link NotFoundException} is thrown.
     *  If there are no permissions to modify the database, the {@link NotFoundException} is thrown.
     *
     *  @param db_name       name of database file from which the include to be removed
     *  @param include       file to be removed from includes
     */

  void remove_include(String db_name, String include) throws SystemException, NotFoundException, NotAllowedException;

    /**
     *  Get include files.
     *
     *  If database <code>db_name</code> is not loaded or there is no such include, the {@link NotFoundException} is thrown.
     *
     *  @param db_name       name of database
     */

  String[] get_includes(String db_name) throws SystemException, NotFoundException;

    /**
     *  Get vector of updated files to be committed.
     *
     *  Return vector of strings containing names of uncommitted database files.
     */

  String[] get_updated_dbs() throws SystemException, NotAllowedException;

    /**
     *  Set commit credentials.
     *
     *  The method sets credentials used by commit() method.
     *
     *  @param user       user name
     *  @param password   user password
     */

  void set_commit_credentials(String user, String password) throws SystemException, NotAllowedException;

    /**
     *  Commit database changes.
     *
     *  Return <b>true</b> if the database changes were successfully saved
     *  and <b>false</b> otherwise.
     *
     *  @param why       log information
     */

  boolean commit(String why) throws SystemException, NotAllowedException;

    /**
     *  Abort database changes.
     *
     *  Return <b>true</b> if the database changes were successfully saved
     *  and <b>false</b> otherwise.
     */

  boolean abort() throws SystemException, NotAllowedException;

    /**
     *  The method returns config object by it's class name and identity.
     *  In case if no such object is found, the method throws exception {@link NotFoundException}.
     *  @param class_name   name of the class where to search object
     *  @param id           object's identity
     */

  ConfigObject get(String class_name, String id) throws SystemException, NotFoundException;

    /**
     *  The method returns array of objects of given class which satisfy to the query expression.
     *  If the query is empty, all objects of given class and it's subclasses are returned.
     *  In case if no such class is found or query has bad syntax, the method throws
     *  exception {@link NotFoundException}.
     *  @param class_name   name of the class where to search objects
     *  @param query        query
     */

  ConfigObject[] get(String class_name, Query query) throws SystemException, NotFoundException;

    /**
     *  The method returns array of objects which are in the path starting from source object
     *  matching to the path query pattern.  In case of problems, the method throws
     *  exception {@link NotFoundException}.
     *  @param from      the source object from which the search of path is started
     *  @param query     query
     */

  ConfigObject[] get(ConfigObject from, Query query) throws SystemException, NotFoundException;

    /**
     *  The method creates new object.
     *
     *  @param db          database file name
     *  @param at          create new object at the same database file where \b 'at' object is located
     *  @param class_name  class name
     *  @param object_id   object identity
     */

  ConfigObject create(String db, ConfigObject at, String class_name, String object_id) throws SystemException, NotFoundException, NotAllowedException;

    /**
     *  The method destroys given object.
     *
     *  @param object   the object to be destroyed
     */

  void destroy(ConfigObject object) throws SystemException, NotFoundException, NotAllowedException;
  
    /**
     *  The method moves existing object to a database.
     *
     *  @param object      the object to be moved
     *  @param at          database file name
     */

  void move(ConfigObject object, String at) throws SystemException, NotFoundException, NotAllowedException;

    /**
     *  The method subscribes on database changes.
     *
     *  @param classes  set of classes on which there is a subscription
     *  @param objects  map of objects on which there is a subscription
     *  @param conf     the configuration object; it is used to invoke {@link Configuration#notify} method in case of changes
     */

  void subscribe(TreeSet<String> classes, Hashtable<String,TreeSet<String>> objects, Configuration conf) throws SystemException;

    /** Removes subscription on changes. */

  void unsubscribe() throws SystemException;


  /**
   *  The method returns class meta information.
   *  @param class_name   name of the class
   *  @param direct_only  return descriptions of all or direct attributes, relationships, super and subclasses
   */

  class_t get(String class_name, boolean direct_only) throws SystemException, NotFoundException;


	/**
	 * Implementation of method Configuration.get_changes()
	 */

  Version[] get_changes() throws SystemException;


	/**
	 * Implementation of method Configuration.get_versions()
	 */

  Version[] get_versions(String since, String until, Version.QueryType type, boolean skip_irrelevant) throws SystemException;

}

package config;

  /**
   *  The <b>ConfigAction</b> interface is used to describe base methods of callback on configuration change.
   *
   *  @author  http://consult.cern.ch/xwho/people/432778
   *  @since   tdaq release 01-08-04
   */

public interface ConfigAction {

    /** Is called when configuration is updated */

  void notify( Change[] changes ) ;


    /** Is called when configuration is loaded */

  void load( ) ;


    /** Is called when configuration is unloaded */

  void unload( ) ;


    /** Is called when object is updated */

  void update( ConfigObject obj, String name ) ;

}

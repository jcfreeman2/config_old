package config;

  /**
   *  The <b>Callback</b> interface is used for receiving "<i>interesting</i>" information describing
   *  database changes. The class that is interested in receiving database changes should to implement
   *  this interface. An object of this class (<i>the callback object</i>) to be used as parameter of
   *  the {@link Subscription#Subscription(Callback, java.lang.Object) Subscription()}
   *  constructor. The subscription object to be used for subscription with {@link Configuration#subscribe(Subscription)}
   *  method invoked on the object of the {@link Configuration} class (<i>the configuration object</i>).
   *  If one modifies the database used by the configuration object and the subscription criteria matches
   *  those changes, then the method <b>process_changes()</b> with the changes description is invoked in
   *  the callback object.
   *
   *  @author  http://consult.cern.ch/xwho/people/432778
   *  @since   online release 00-21-00
   */

public interface Callback {

    /**
     *  Invoked when the requested changes appear.
     *
     *  @param changes    vector of changes
     *  @param parameter  user-defined parameter passed during subscription
     */

  void process_changes(Change[] changes, Object parameter);

}

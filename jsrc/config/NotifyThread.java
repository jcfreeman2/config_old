package config;

import java.lang.Thread;

    /**
     *  The <b>NotifyThread</b> class provides interfaces for notification about changes.
     *  The method calls config.Configuration.notify() method in a new thread.
     *  This is done to avoid possible blocking of caller waiting return from user callback.
     *  It is used by the config plug-ins and should not be used directly by the user.
     */

public class NotifyThread extends Thread {
  config.Configuration m_config;
  config.Change[] m_changes;

    /** Build new thread from configuration object and list of changes. */

  public NotifyThread(config.Configuration config, config.Change[] changes) {
    this.m_config = config;
    this.m_changes = changes;
  }

    /** Run new thread to call user's callbacks registered on the configuration object. */

  public void run() {
    m_config.notify(m_changes);
  }
}

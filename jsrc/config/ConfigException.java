package config;

/**
 * Used as a base class for all the Exceptions thrown by the config package.
 * 
 * @author http://consult.cern.ch/xwho/people/432778
 * @since 17/02/21
 */

@SuppressWarnings("serial")
public class ConfigException extends ers.Issue
  {

    public ConfigException(String string)
      {
        super(string);
      }

    public ConfigException(String string, Exception ex)
      {
        super(string, ex);
      }

    public ConfigException(Exception ex)
      {
        super(ex);
      }

  }
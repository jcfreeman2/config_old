package config;

/**
 * Thrown to indicate that the query syntax is bad.
 * 
 * @author http://consult.cern.ch/xwho/people/432778
 * @since online release 00-21-02
 */

@SuppressWarnings("serial")
public class BadQueryException extends ConfigException
  {

    public BadQueryException(String message)
      {
        super(message);
      }

    public BadQueryException(String message, Exception cause)
      {
        super(message, cause);
      }

    public BadQueryException(Exception cause)
      {
        super(cause);
      }
  }
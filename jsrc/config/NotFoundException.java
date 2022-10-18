package config;

/**
 * Thrown to indicate that the requested item (class, object, attribute, etc.)
 * does not exist.
 * 
 * @author http://consult.cern.ch/xwho/people/432778
 * @since 28/01/04
 */

@SuppressWarnings("serial")
public class NotFoundException extends ConfigException
  {

    public NotFoundException(String message)
      {
        super(message);
      }

    public NotFoundException(String message, Exception cause)
      {
        super(message, cause);
      }

    public NotFoundException(Exception cause)
      {
        super(cause);
      }

  }
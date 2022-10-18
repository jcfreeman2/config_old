package config;

/**
 * Thrown to indicate that the requested action is not allowed to given user.
 * 
 * @author http://consult.cern.ch/xwho/people/432778
 * @since 28/01/04
 */

@SuppressWarnings("serial")
public class NotAllowedException extends ConfigException
  {

    public NotAllowedException(String message)
      {
        super(message);
      }

    public NotAllowedException(String message, Exception cause)
      {
        super(message, cause);
      }

    public NotAllowedException(Exception cause)
      {
        super(cause);
      }

  }
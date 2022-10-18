package config;

/**
 * Thrown to indicate that the object is not valid, for example after database reload, update or wrong initialisation.
 *
 * @author http://consult.cern.ch/xwho/people/432778
 * @since 06/04/04
 */

@SuppressWarnings("serial")
public class NotValidException extends ConfigException
  {

    public NotValidException(String message)
      {
        super(message);
      }

    public NotValidException(String message, Exception cause)
      {
        super(message, cause);
      }

    public NotValidException(Exception cause)
      {
        super(cause);
      }

  }
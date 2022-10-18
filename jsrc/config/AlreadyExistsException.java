package config;

/**
 * Thrown to indicate that the an created item (an object, a file) already
 * exists.
 *
 * @author http://consult.cern.ch/xwho/people/432778
 * @since 31/03/04
 */

@SuppressWarnings("serial")
public class AlreadyExistsException extends ConfigException
  {

    public AlreadyExistsException(String message)
      {
        super(message);
      }

    public AlreadyExistsException(String message, Exception cause)
      {
        super(message, cause);
      }

    public AlreadyExistsException(Exception cause)
      {
        super(cause);
      }
  }
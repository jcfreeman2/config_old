package config;

/**
 * Thrown to indicate that there is a system exception (communication failure,
 * file access problem, etc).
 * 
 * @author http://consult.cern.ch/xwho/people/432778
 * @since 06/04/04
 */

@SuppressWarnings("serial")
public class SystemException extends ConfigException
  {

    public SystemException(String message)
      {
        super(message);
      }

    public SystemException(String message, Exception cause)
      {
        super(message, cause);
      }

    public SystemException(Exception cause)
      {
        super(cause);
      }

  }
package config;

/**
 * Reports generic config problem like wrong usage of parameters, algorithm
 * problems, etc.
 * 
 * @author http://consult.cern.ch/xwho/people/432778
 * @since 17/02/21
 */

@SuppressWarnings("serial")
public class GenericException extends ConfigException
  {

    public GenericException(String string)
      {
        super(string);
      }

    public GenericException(String string, Exception ex)
      {
        super(string, ex);
      }

    public GenericException(Exception ex)
      {
        super(ex);
      }

  }

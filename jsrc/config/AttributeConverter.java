package config;

  /**
   *  The interface defines methods for an attribute converter object.
   *
   *  @author  http://consult.cern.ch/xwho/people/432778
   *  @since   june-2005
   */

public interface AttributeConverter
{
   /**
     *  The method performs conversion of the value.
     *
     *  @param value      the value which is converted
     *  @param db         the database configuration object
     *  @param obj        the object, which attribute value is converted
     *  @param attr_name  the name of attribute, which value is converted
     */

  Object convert(Object value, Configuration db, ConfigObject obj, String attr_name) throws config.GenericException, config.NotFoundException, config.NotValidException, config.SystemException;


   /**
     *  The method returns class of objects modified by the converter.
     */

  Class get_class();
}

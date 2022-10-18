package config;

  /**
   *  The <b>Query</b> class is used to create query expression.
   *  For the moment a query is the OKS query string. In future the class
   *  will be extended to allow dynamic query creating which will potentially
   *  allow to work with others database implementations.
   *
   *  @author  http://consult.cern.ch/xwho/people/432778
   *  @since   online release 00-21-02
   */

public class Query {

  private String p_query;

    /** Returns query string. */

  public String get_query_string() {
    return p_query;
  }

    /** Constructor to build an empty query. */

  public Query() {
    p_query = "";
  }

    /**
     *  Constructor to build query from string.
     *  @param query query string
     */

  public Query(String query) throws BadQueryException {
    p_query = query;
  }

}

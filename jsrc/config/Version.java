package config;

import java.time.Instant;

/**
 * The <b>ConfigVersion</b> class is used to describe OKS GIT repository
 * version.
 * <p>
 * An objects are created by the Configuration.get_versions() and
 * Configuration.get_changes() methods.
 *
 * @author http://consult.cern.ch/xwho/people/432778
 * @since tdaq-09-01-00
 */

public class Version {
	String m_id;
	String m_user;
	Instant m_timestamp;
	String m_comment;
	String[] m_files;

	public Version(String id, String user, long ts, String comment, String[] files) {
		m_id = id;
		m_user = user;
		m_timestamp = Instant.ofEpochSecond(ts);
		m_comment = comment;
		m_files = files;
	}

	/**
	 * The supported version query types used by Configuration.
	 */

	public enum QueryType {
		/** query versions by dates */
		query_by_date,

		/** query versions by unique ID (git SHA) */
		query_by_id,

		/** query versions by tags */
		query_by_tag
	};

	/**
	 * Return version unique ID is a repository hash (GIT SHA).
	 */

	public String get_id() {
		return m_id;
	}

	/**
	 * Return user who made a commit.
	 */
	public String get_user() {
		return m_user;
	}

	/**
	 * Return timestamp (in seconds since Epoch) when commit was made.
	 */
	public Instant get_timestamp() {
		return m_timestamp;
	}

	/**
	 * Return commit comment.
	 */
	public String get_comment() {
		return m_comment;
	}

	/**
	 * Return files modified in this version.
	 */
	public String[] get_files() {
		return m_files;
	}

}

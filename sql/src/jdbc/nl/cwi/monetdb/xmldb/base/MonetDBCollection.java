package nl.cwi.monetdb.xmldb.base;

import org.xmldb.api.base;


/**
 * A Collection represents a collection of Resources stored within an
 * XML database. An XML database MAY expose collections as a
 * hierarchical set of parent and child collections.
 * <p />
 * MonetDB/XQuery at the moment exposes no collection at all.
 * <p />
 * A Collection provides access to the Resources stored by the
 * Collection and to Service instances that can operate against the
 * Collection and the Resources stored within it.  The Service mechanism
 * provides the ability to extend the functionality of a Collection in
 * ways that allows optional functionality to be enabled for the
 * Collection. 
 *
 * @author Fabian Groffen <Fabian.Groffen@cwi.nl>
 */
public class MonetDBCollection implements Collection {
	private Service[] knownServices;
	private final MonetDBDatabase monet;
	private boolean closed;

	/**
	 * Constructs a new MonetDB Collection and initialises its
	 * knownService array.
	 */
	MonetDBCollection(MonetDBDatabase db) {
		monet = db;
		// initially fill the knownServices array
		knownServices = { new MonetDBXQueryService(this) };
		
		closed = false;
	}
	
	/**
	 * Returns the name associated with the Collection instance.
	 *
	 * @return the name of the object.
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.<br />
	 */
	String getName() throws XMLDBException {
		return("MonetDBCollection");
	}

	/**
	 * Provides a list of all services known to the collection.  If no
	 * services are known an empty list is returned.
	 * <p />
	 * In MonetDB/XQuery's case we return a list with an XQueryService
	 * implementation.
	 *
	 * @return An array of registered Service implementations.
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.<br />
	 *  ErrorCodes.COLLECTION_CLOSED if the close method has been called
	 *  on the Collection<br />
	 */
	Service[] getServices() throws XMLDBException {
		if (closed) throw new XMLDBException(ErrorCodes.COLLECTION_CLOSED);

		// We should return a list with all supported Services.
		return(knownServices.clone());
	}

	/**
	 * Returns a Service instance for the requested service name and
	 * version.  If no Service exists for those parameters a null value
	 * is returned.
	 *
	 * @param name Description of Parameter
	 * @param version Description of Parameter
	 * @return the Service instance or null if no Service could be found.
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.<br />
	 *  ErrorCodes.COLLECTION_CLOSED if the close method has been called
	 *  on the Collection<br />
	 */
	Service getService(String name, String version) throws XMLDBException {
		if (closed) throw new XMLDBException(ErrorCodes.COLLECTION_CLOSED);
		
		// Do it nice, use reflection here and iterate over all known
		// Services.  Get their name and version using getName() and
		// getVersion and do the comparison.
		for (int i = 0; i < knownServices.length; i++) {
			if (knownServices[i].getName().equals(name) &&
					knownServices[i].getVersion().equals(version))
				return(knownServices[i]);
		}

		// finally, if not found, return null
		return(null);
	}

	/**
	 * Returns the parent collection for this collection or null if no
	 * parent collection exists.
	 *
	 * @return the parent Collection instance.
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.<br />
	 *  ErrorCodes.COLLECTION_CLOSED if the close method has been called
	 *  on the Collection<br />
	 */
	Collection getParentCollection() throws XMLDBException {
		if (closed) throw new XMLDBException(ErrorCodes.COLLECTION_CLOSED);
		
		// I do it quick 'n' dirty for now.  There exists no recursion,
		// so there is never a parent.
		return(null);
	}

	/**
	 * Returns the number of child collections under this Collection or
	 * 0 if no child collections exist.
	 *
	 * @return the number of child collections.
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.<br />
	 *  ErrorCodes.COLLECTION_CLOSED if the close method has been called
	 *  on the Collection<br />
	 */
	int getChildCollectionCount() throws XMLDBException {
		// again quick 'n' dirty (see above)
		return(0);
	}

	/**
	 * Returns a list of collection names naming all child collections
	 * of the current collection.  If no child collections exist an
	 * empty list is returned.
	 *
	 * @return an array containing collection names for all child
	 *         collections.
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.<br />
	 *  ErrorCodes.COLLECTION_CLOSED if the close method has been called
	 *  on the Collection<br />
	 */
	String[] listChildCollections() throws XMLDBException {
		// quick 'n' dirty! (see above)
		return(new String[0]);
	}

	/**
	 * Returns a Collection instance for the requested child collection
	 * if it exists.
	 *
	 * @param name the name of the child collection to retrieve.
	 * @return the requested child collection or null if it couldn't be found.
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.<br />
	 *  ErrorCodes.COLLECTION_CLOSED if the close method has been called
	 *  on the Collection<br />
	 */
	Collection getChildCollection(String name) throws XMLDBException {
		// we don't have children, so we always return null regardless
		// the input
		return(null);
	}

	/**
	 * Returns the number of resources currently stored in this
	 * collection or 0 if the collection is empty.
	 *
	 * @return the number of resource in the collection.
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.<br />
	 *  ErrorCodes.COLLECTION_CLOSED if the close method has been called
	 *  on the Collection<br />
	 */
	int getResourceCount() throws XMLDBException {
		// We cannot know upfront how many tuples there are to come
		// using JDBC.  I don't know...
		return(0);
	}

	/**
	 * Returns a list of the ids for all resources stored in the
	 * collection.
	 *
	 * @return a string array containing the names for all Resources in
	 * the collection.
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.<br />
	 *  ErrorCodes.COLLECTION_CLOSED if the close method has been called
	 *  on the Collection<br />
	 */
	String[] listResources() throws XMLDBException {
		// somehow resources have IDs...  I'm affraid we have to take
		// the hash of the tuples here or something.
		return(new String[0]);
	}

	/**
	 * Creates a new empty Resource with the provided id.  The type of
	 * Resource returned is determined by the type parameter.  The
	 * XML:DB API currently defines "XMLResource" and "BinaryResource"
	 * as valid resource types.  The id provided must be unique within
	 * the scope of the collection.  If id is null or its value is empty
	 * then an id is generated by calling createId().  The Resource
	 * created is not stored to the database until storeResource() is
	 * called.
	 *
	 * @param id the unique id to associate with the created Resource.
	 * @param type the Resource type to create.
	 * @return an empty Resource instance.    
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.<br />
	 *  ErrorCodes.UNKNOWN_RESOURCE_TYPE if the type parameter is not a
	 *  known Resource type.
	 *  ErrorCodes.COLLECTION_CLOSED if the close method has been called
	 *  on the Collection<br />
	 */
	Resource createResource(String id, String type) throws XMLDBException {
		// we don't have updateable resultsets (yet)
		throw new XMLDBException(ErrorCodes.VENDOR_ERROR, "Operation not supported, sorry.");
	}

	/**
	 * Removes the Resource from the database.
	 *
	 * @param res the resource to remove.
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.<br />
	 *  ErrorCodes.INVALID_RESOURCE if the Resource is not valid.<br />
	 *  ErrorCodes.NO_SUCH_RESOURCE if the Resource is not known to this
	 *  Collection.
	 *  ErrorCodes.COLLECTION_CLOSED if the close method has been called
	 *  on the Collection<br />
	 */
	void removeResource(Resource res) throws XMLDBException {
		// we don't have updateable resultsets (yet)
		throw new XMLDBException(ErrorCodes.VENDOR_ERROR, "Operation not supported, sorry.");
	}

	/**
	 * Stores the provided resource into the database. If the resource
	 * does not already exist it will be created. If it does already
	 * exist it will be updated.
	 *
	 * @param res the resource to store in the database.
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.<br />
	 *  ErrorCodes.INVALID_RESOURCE if the Resource is not valid.
	 *  ErrorCodes.COLLECTION_CLOSED if the close method has been called
	 *  on the Collection<br />
	 */
	void storeResource(Resource res) throws XMLDBException {
		// we don't have updateable resultsets (yet)
		throw new XMLDBException(ErrorCodes.VENDOR_ERROR, "Operation not supported, sorry.");
	}

	/**
	 * Retrieves a Resource from the database. If the Resource could not
	 * be located a null value will be returned.
	 *
	 * @param id the unique id for the requested resource.
	 * @return The retrieved Resource instance.
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.<br />    
	 *  ErrorCodes.COLLECTION_CLOSED if the close method has been called
	 *  on the Collection<br />
	 */
	Resource getResource(String id) throws XMLDBException {
		// do something like return the row requested
		// currently: do nothing
		return(null);
	}

	/**
	 * Creates a new unique ID within the context of the <code>Collection</code>
	 *
	 * @return the created id as a string.
	 * @throws XMLDBException with expected error codes.<br />
	 *  <code>ErrorCodes.VENDOR_ERROR</code> for any vendor
	 *  specific errors that occur.<br />
	 *  <code>ErrorCodes.COLLECTION_CLOSED</code> if the <code>close</code> 
	 *  method has been called on the <code>Collection</code><br />
	 */
	String createId() throws XMLDBException;

	/**
	 * Returns true if the  <code>Collection</code> is open false otherwise.
	 * Calling the <code>close</code> method on 
	 * <code>Collection</code> will result in <code>isOpen</code>
	 * returning false. It is not safe to use <code>Collection</code> instances
	 * that have been closed.
	 *
	 * @return true if the <code>Collection</code> is open, false otherwise.
	 * @throws XMLDBException with expected error codes.<br />
	 *  <code>ErrorCodes.VENDOR_ERROR</code> for any vendor
	 *  specific errors that occur.<br />
	 */
	boolean isOpen() throws XMLDBException {
		return(!closed);
	}

	/**
	 * Releases all resources consumed by the <code>Collection</code>. 
	 * The <code>close</code> method must
	 * always be called when use of a <code>Collection</code> is complete. It is
	 * not safe to use a  <code>Collection</code> after the <code>close</code>
	 * method has been called.
	 *
	 * @throws XMLDBException with expected error codes.<br />
	 *  <code>ErrorCodes.VENDOR_ERROR</code> for any vendor
	 *  specific errors that occur.<br />
	 */
	void close() throws XMLDBException {
		// perhaps a stmt.close();
		closed = true;
	}
}


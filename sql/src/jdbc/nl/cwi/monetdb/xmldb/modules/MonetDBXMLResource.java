package nl.cwi.monetdb.xmldb.modules;

import org.xmldb.api.base.*;
import org.xmldb.api.modules.*;

import org.w3c.dom.*;
import org.xml.sax.*;

/**
 * Provides access to XML resources stored in the database.  An
 * XMLResource can be accessed either as text XML or via the DOM or SAX
 * APIs.<br />
 * <br />
 * The default behavior for getContent and setContent is to work with
 * XML data as text so these methods work on String content.
 */
public class MonetDBXMLResource implements XMLResource {

	//== interface org.xmldb.api.base.Resource

	/**
	 * Returns the Collection instance that this resource is associated
	 * with.  All resources must exist within the context of a
	 * Collection.
	 *
	 * @return the Collection associated with this Resource
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.
	 */
	public Collection getParentCollection() throws XMLDBException {
		// should return MonetDBCollection thingher
		return(myParent);
	}

	/**
	 * Returns the unique id for this Resource or null if the Resource
	 * is anonymous.  The Resource will be anonymous if it is obtained
	 * as the result of a query.
	 *
	 * @returns the id for the Resource or null if no id exists.
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.
	 */
	public String getId() throws XMLDBException {
		// we're anonymous if we haven't got a Collection parent
		if (myParent != null) {
			return(toString());
		} else {
			return(null);
		}
	}

	/**
	 * Returns the resource type for this Resource.<br />
	 * <br />
	 * XML:DB defined resource types are:<br />
	 * XMLResource - all XML data stored in the database
	 * BinaryResource - Binary blob data stored in the database
	 *
	 * @return the resource type for the Resource.
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.
	 */
	public String getResourceType() throws XMLDBException {
		// RESOURCE_TYPE is defined in the interface XMLResource
		return(RESOURCE_TYPE);
	}

	/**
	 * Retrieves the content from the resource.  The type of the content
	 * varies depending what type of resource is being used.
	 *
	 * @return the content of the resource as XML String
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.
	 */
	public Object getContent() throws XMLDBException {
		// we return as String
		return(content);
	}

	/**
	 * Sets the content for this resource.  The type of content that can
	 * be set depends on the type of resource being used.
	 *
	 * @param value the content value to set for the Resource
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.
	 */
	public void setContent(Object value) throws XMLDBException {
		// can we do this without penalty?
		content = value.toString();
	}

	//== end interface Resource
	
	//== Interface org.xmldb.api.modules.XMLResource
	
	/**
	 * Returns the unique id for the parent document to this Resource
	 * or null if the Resource does not have a parent document.
	 * getDocumentId() is typically used with Resource instances
	 * retrieved using a query.  It enables accessing the parent
	 * document of the Resource even if the Resource is a child node of
	 * the document.  If the Resource was not obtained through a query
	 * then getId() and getDocumentId() will return the same id.
	 *
	 * @return the id for the parent document of this Resource or null
	 *         if there is no parent document for this Resource.
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.
	 */
	public String getDocumentId() throws XMLDBException {
		// hmmm... oops... I dunno
		return(null);
	}

	/**
	 * Returns the content of the Resource as a DOM Node.
	 *
	 * @return The XML content as a DOM Node
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.
	 */
	public org.w3c.dom.Node getContentAsDOM() throws XMLDBException {
		// do the terrible DOM thing...
		// maar nu even niet! (TODO!)
		return(null);
	}

	/**
	 * Sets the content of the Resource using a DOM Node as the source.
	 *
	 * @param content The new content value
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.<br />
	 *  ErrorCodes.INVALID_RESOURCE if the content value provided is
	 *  null.<br />
	 *  ErrorCodes.WRONG_CONTENT_TYPE if the content provided in not a
	 *  valid DOM Node.
	 */
	public void setContentAsDOM(org.w3c.dom.Node content) throws XMLDBException {
		if (content == null) throw
			new XMLDBException(ErrorCodes.INVALID_RESOURCE, "(null)");

		// we can't do anything yet
		throw new XMLDBException(ErrorCodes.VENDOR_ERROR, "not implemented");
	}

	/**
	 * Allows you to use a ContentHandler to parse the XML data from the
	 * database for use in an application.
	 *
	 * @param handler the SAX ContentHandler to use to handle the
	 *                Resource content.
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.<br />
	 *  ErrorCodes.INVALID_RESOURCE if the ContentHandler value provided
	 *  is null.
	 */
	public void getContentAsSAX(org.xml.sax.ContentHandler handler)
		throws XMLDBException
	{
		if (handler == null) throw
			new XMLDBException(ErrorCodes.INVALID_RESOURCE, "(null)");

		// we can't do anything yet
		throw new XMLDBException(ErrorCodes.VENDOR_ERROR, "not implemented");
	}

	/**
	 * Sets the content of the Resource using a SAX ContentHandler.
	 *
	 * @return a SAX ContentHandler that can be used to add content into
	 *         the Resource.
	 * @throws XMLDBException with expected error codes.<br />
	 *  ErrorCodes.VENDOR_ERROR for any vendor specific errors that
	 *  occur.
	 */
	public org.xml.sax.ContentHandler setContentAsSAX() throws XMLDBException {
		// I have NO idea
		throw new XMLDBException(ErrorCodes.VENDOR_ERROR, "not implemented");
	}

	/**
	 * Sets a SAX feature that will be used when this XMLResource  is
	 * used to produce SAX events (through the getContentAsSAX()
	 * method).
	 *
	 * @param feature Feature name. Standard SAX feature names are
	 *                documented at http://sax.sourceforge.net/.
	 * @param value Set or unset feature
	 * @throws org.xml.sax.SAXNotRecognizedException
	 * @throws org.xml.sax.SAXNotSupportedException
	 */
	public void setSAXFeature(String feature, boolean value)
		throws org.xml.sax.SAXNotRecognizedException,
				org.xml.sax.SAXNotSupportedException
	{
		// do nothing
	}

	/**
	 * Returns current setting of a SAX feature that will be used when
	 * this XMLResource is used to produce SAX events (through the
	 * getContentAsSAX() method)
	 *
	 * @param feature Feature name. Standard SAX feature names are
	 *                documented at http://sax.sourceforge.net/.
	 * @return whether the feature is set
	 * @throws org.xml.sax.SAXNotRecognizedException
	 * @throws org.xml.sax.SAXNotSupportedException
	 */
	public boolean getSAXFeature(String feature)
		throws org.xml.sax.SAXNotRecognizedException,
				org.xml.sax.SAXNotSupportedException
	{
		// eh?
		return(false);
	}
}


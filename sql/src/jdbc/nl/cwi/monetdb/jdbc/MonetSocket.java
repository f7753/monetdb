package nl.cwi.monetdb.jdbc;

import java.io.*;
import java.net.*;

/**
 * A Socket for communicating with the Monet database
 * <br /><br />
 * This MonetSocket performs basic operations like sending the server a message
 * and/or receiving a line from it. A small interpretation of all what is read
 * is done, to supply some basic tools for the using classes.<br />
 * For each read line, it is determined what type of line it is according to the
 * MonetDB MAPI protocol. This results in a line to be PROMPT, HEADER, RESULT,
 * ERROR or UNKNOWN. Use the getLineType() function to retrieve the type of the
 * last line read.
 * <br /><br />
 * For debugging purposes a socket level debugging is implemented where each and
 * every interaction to and from the Monet server is logged to a file on disk.
 * Incomming messages are prefixed by "&lt;&lt;", outgoing messages by
 * "&gt;&gt;".
 *
 * @author Fabian Groffen <Fabian.Groffen@cwi.nl>
 * @version 1.2 (part of MonetDB JDBC beta release)
 */
class MonetSocket {
	/** Reader from the Socket */
	private BufferedReader fromMonet;
	/** Writer to the Socket */
	private BufferedWriter toMonet;
	/** The TCP Socket to Mserver */
	protected Socket con;

	/** Whether we are debugging or not */
	protected boolean debug = false;
	/** The Writer for the debug log-file */
	protected FileWriter log;

	/** The type of the last line read */
	protected int lineType;

	/** "there is currently no line" is represented by EMPTY */
	final static int EMPTY = 0;
	/** a line starting with ! indicates ERROR */
	final static int ERROR = 1;
	/** a line starting with # indicates HEADER */
	final static int HEADER = 2;
	/** a line starting with [ indicates RESULT */
	final static int RESULT = 3;
	/** a line which matches the pattern of prompt1 is a PROMPT1 */
	final static int PROMPT1 = 4;
	/** a line which matches the pattern of prompt2 is a PROMPT2 */
	final static int PROMPT2 = 5;

	MonetSocket(String host, int port) throws IOException {
		con = new Socket(host, port);
		fromMonet = new BufferedReader(
			new InputStreamReader(con.getInputStream()));
		toMonet = new BufferedWriter(
			new OutputStreamWriter(con.getOutputStream()));
	}

	protected MonetSocket(Socket con) {
		this.con = con;
	}

	/**
	 * enables logging to a file what is read and written from and to Monet
	 *
	 * @param filename the name of the file to write to
	 * @throws IOException if the file could not be opened for writing
	 */
	public void debug(String filename) throws IOException {
		log = new FileWriter(filename);
		debug = true;
	}

	/**
	 * write puts the given string on the stream as is, where is no newline
	 * appended, and the stream will not be flushed after the write.
	 * To flush the stream use flush(), or use writeln().
	 *
	 * @param data the data to write to the stream
	 * @throws IOException if writing to the stream failed
	 * @see #flush()
	 * @see #writeln(String data)
	 */
	public synchronized void write(String data) throws IOException {
		toMonet.write(data);
		// reset the lineType variable, since we've sent data now and the last
		// line isn't valid anymore
		lineType = EMPTY;

		// it's a bit nasty if an exception is thrown from the log,
		// but ignoring it can be nasty as well, so it is decided to
		// let it go so there is feedback about something going wrong
		if (debug) log.write("<< " + data);
	}

	/**
	 * flushes the stream to monet, forcing all data in the buffer to be
	 * actually written to the stream.
	 *
	 * @throws IOException if writing to the stream failed
	 */
	public synchronized void flush() throws IOException {
		toMonet.flush();
		// it's a bit nasty if an exception is thrown from the log,
		// but ignoring it can be nasty as well, so it is decided to
		// let it go so there is feedback about something going wrong
		if (debug) log.flush();
	}

	/**
	 * writeln puts the given string plus a new line character on the stream
	 * and flushes the stream afterwards so the data will actually be sent
	 *
	 * @param data the data to write to the stream
	 * @throws IOException if writing to the stream failed
	 */
	public synchronized void writeln(String data) throws IOException {
		write(data + "\n");
		flush();
	}

	/**
	 * readLine reads one line terminated by a newline character and returns
	 * it without the newline character. This operation can be blocking if there
	 * is no information available (yet)
	 *
	 * @return a string representing the next line from the stream
	 * @throws IOException if reading from the stream fails
	 */
	public synchronized String readLine() throws IOException {
		String line;
		do {
			line = fromMonet.readLine();

			// it's a bit nasty if an exception is thrown from the log,
			// but ignoring it can be nasty as well, so it is decided to
			// let it go so there is feedback about something going wrong
			if (debug) {
				log.write(">> " + line + "\n");
				log.flush();
			}
		} while (line != null && line.length() == 0);

		if (line != null) {
			switch (line.charAt(0)) {
				case '!': lineType = ERROR; break;
				case '#': lineType = HEADER; break;
				case '[': lineType = RESULT; break;
				default:
					if (MonetDriver.prompt1.equals(line)) {
						lineType = PROMPT1;	// prompt1 found
					} else if (MonetDriver.prompt2.equals(line)) {
						lineType = PROMPT2;	// prompt2 found
					} else {
						lineType = EMPTY;	// unknown :-(
					}
				break;
			}
		} else {
			throw new IOException("End of stream reached");
		}

		return(line);
	}

	/**
	 * getLineType returns the type of the last line read
	 *
	 * @return an integer representing the kind of line this is, one of the
	 *         following constants: EMPTY, HEADER, ERROR, PROMPT, RESULT
	 */
	public int getLineType() {
		return(lineType);
	}

	/**
	 * Reads up till the MonetDB prompt, indicating the server is ready for a
	 * new command. All read data is discarded.<br />
	 * If the last line read by readLine() was a prompt, this method will
	 * immediately return.
	 * <br /><br />
	 * If there are errors present in the lines that are read, then they are put
	 * in one string and returned <b>after</b> the prompt has been found. If no
	 * errors are present, null will be returned.
	 *
	 * @return a string containing error messages, or null if there aren't any
	 * @throws IOException if an IO exception occurs while talking to the server
	 */
	public synchronized String waitForPrompt() throws IOException {
		String ret = "", tmp;
		while (getLineType() != PROMPT1) {
			if ((tmp = readLine()) == null)
				throw new IOException("Connection to server lost!");
			if (getLineType() == ERROR) ret += "\n" + tmp.substring(1);
		}
		return(ret == "" ? null : ret.trim());
	}

	/**
	 * disconnect closes the streams and socket connected to the Monet server
	 * if possible. If an error occurs during disconnecting it is ignored.
	 */
	public synchronized void disconnect() {
		try {
			fromMonet.close();
			toMonet.close();
			con.close();
			if (debug) log.close();
		} catch (IOException e) {
			// ignore it
		}
	}

	/**
	 * destructor called by garbage collector before destroying this object
	 * tries to disconnect the Monet connection if it has not been disconnected
	 * already
	 */
	protected void finalize() {
		disconnect();
	}
}

package nl.cwi.monetdb.jdbc;

import java.io.*;
import java.nio.*;
import java.net.*;

/**
 * A Socket for communicating with the Monet database in block mode
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
 * <br /><br />
 * This implementation of MonetSocket uses block mode on the mapi protocol in
 * order to circumvent the drawbacks of line mode. It allows sending a multi
 * line query, and should be less intensive for the server.
 *
 * @author Fabian Groffen <Fabian.Groffen@cwi.nl>
 * @version 0.2 (part of MonetDB JDBC beta release)
 */
class MonetSocketBlockMode extends MonetSocket {
	/** Stream from the Socket for reading */
	private InputStream fromMonetRaw;
	/** Stream from the Socket for writing */
	private OutputStream toMonetRaw;

	/** A buffer which holds the blocks read */
	private StringBuffer readBuffer;
	/** The number of available bytes to read */
	private int readState = 0;

	/** A ByteBuffer for performing a possible swab on an integer for reading */
	private ByteBuffer inputBuffer;
	/** A ByteBuffer for performing a possible swab on an integer for sending */
	private ByteBuffer outputBuffer;

	/** The maximum size of the chunks we fetch data from the stream with */
	private final static int capacity = 1024;

	MonetSocketBlockMode(String host, int port) throws IOException {
		super(new Socket(host, port));

		fromMonetRaw = new BufferedInputStream(con.getInputStream());
		toMonetRaw = new BufferedOutputStream(con.getOutputStream());

		outputBuffer = ByteBuffer.allocateDirect(4);
		// put the buffer in native byte order
		outputBuffer.order(ByteOrder.nativeOrder());

		inputBuffer = ByteBuffer.allocateDirect(4);
		// leave the buffer byte-order as is, it can be modified later
		// by using setByteOrder()
		readBuffer = new StringBuffer();
	}

	/**
	 * write puts the given string on the stream as is.
	 * The stream will not be flushed after the write.
	 * To flush the stream use flush(), or use writeln().
	 *
	 * @param data the data to write to the stream
	 * @throws IOException if writing to the stream failed
	 * @see #flush()
	 * @see #writeln(String data)
	 */
	public synchronized void write(String data) throws IOException {
		write(data.getBytes());
	}

	/**
	 * Writes the given bytes to the stream
	 *
	 * @param data the bytes to be written
	 * @throws IOException if writing to the stream failed
	 */
	public synchronized void write(byte[] data) throws IOException {
		toMonetRaw.write(data);

		// reset the lineType variable, since we've sent data now and the last
		// line isn't valid anymore
		lineType = EMPTY;

		// it's a bit nasty if an exception is thrown from the log,
		// but ignoring it can be nasty as well, so it is decided to
		// let it go so there is feedback about something going wrong
		// it's a bit nasty if an exception is thrown from the log,
		// but ignoring it can be nasty as well, so it is decided to
		// let it go so there is feedback about something going wrong
		if (debug) {
			log.write("<< " + new String(data));
		}
	}

	/**
	 * flushes the stream to monet, forcing all data in the buffer to be
	 * actually written to the stream.
	 *
	 * @throws IOException if writing to the stream failed
	 */
	public synchronized void flush() throws IOException {
		toMonetRaw.flush();

		// it's a bit nasty if an exception is thrown from the log,
		// but ignoring it can be nasty as well, so it is decided to
		// let it go so there is feedback about something going wrong
		// it's a bit nasty if an exception is thrown from the log,
		// but ignoring it can be nasty as well, so it is decided to
		// let it go so there is feedback about something going wrong
		if (debug) {
			log.flush();
		}
	}

	private final static byte[] BLK_TERMINATOR =
		{(byte)0, (byte)0, (byte)0, (byte)0};
	/**
	 * writeln puts the given string on the stream
	 * and flushes the stream afterwards so the data will actually be sent
	 *
	 * @param data the data to write to the stream
	 * @throws IOException if writing to the stream failed
	 */
	public synchronized void writeln(String data) throws IOException {
		// write the length of this block
		outputBuffer.rewind();
		outputBuffer.putInt(data.length() + 1);

		byte[] len = new byte[4];
		outputBuffer.rewind();
		outputBuffer.get(len);

		write(len);
		write(data + "\n");

		// the server wants an empty int as termination, as it seems
		write(BLK_TERMINATOR);
		flush();
	}

	/**
	 * Reads up to count bytes from the stream, and returns them in a byte array
	 *
	 * @param data a byte array, which should be filled with data from the stream
	 * @return the number of bytes actually read, never less than zero
	 * @throws IOException if some IO error occurs
	 */
	public synchronized int read(byte[] data) throws IOException {
		// read the data
		int size = fromMonetRaw.read(data);
		// note: this catches also end of stream (-1)
		if (size == -1) throw
			new IOException("End of stream reached");

		// it's a bit nasty if an exception is thrown from the log,
		// but ignoring it can be nasty as well, so it is decided to
		// let it go so there is feedback about something going wrong
		if (debug) {
			log.write(">> " + (new String(data)).substring(0, size) + "\n");
			log.flush();
		}

		return(size);
	}

	/**
	 * readLine reads one line terminated by a newline character and returns
	 * it without the newline character. This operation can be blocking if there
	 * is no information available (yet)
	 *
	 * @return a string representing the next line from the stream or null if
	 *         the stream is closed and no data is available (end of stream)
	 * @throws IOException if reading from the stream fails
	 */
	public synchronized String readLine() throws IOException {
		String line;
		int nl;
		while ((nl = readBuffer.indexOf("\n")) == -1) {
			// not found, fetch us some more data
			// start reading a new block of data if appropriate
			if (readState == 0) {
				// read next four bytes (int)
				byte[] data = new byte[4];
				int size = read(data);
				if (size < 4) throw new AssertionError("Illegal start of block");

				// start with a new block
				inputBuffer.rewind();
				inputBuffer.put(data);

				// get the int-value and store it in the state
				inputBuffer.rewind();
				readState = inputBuffer.getInt();
			}
			// 'continue' fetching current block
			byte[] data = new byte[Math.min(capacity, readState)];
			int size = read(data);

			// update the state
			readState -= size;

			// append the stuff to the buffer; let String do the charset
			// conversion stuff
			readBuffer.append((new String(data)).substring(0, size));
		}
		// fill line, excluding newline
		line = readBuffer.substring(0, nl);

		// remove from the buffer, including newline
		readBuffer.delete(0, nl + 1);

		if (line.length() != 0) {
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
			// I really think empty lines should not be sent by the server
			throw new IOException("MonetBadTasteException: it sent us an empty line");
		}
		return(line);
	}

	/**
	 * Sets the byte-order to reading data from the server. By default the
	 * byte order is big-endian or network order.
	 *
	 * @param order either ByteOrder.BIG_ENDIAN or ByteOrder.LITTLE_ENDIAN
	 */
	public void setByteOrder(ByteOrder order) {
		inputBuffer.order(order);
	}

	/**
	 * disconnect closes the streams and socket connected to the Monet server
	 * if possible. If an error occurs during disconnecting it is ignored.
	 */
	public synchronized void disconnect() {
		try {
			fromMonetRaw.close();
			toMonetRaw.close();
			con.close();
			if (debug) log.close();
		} catch (IOException e) {
			// ignore it
		}
	}
}

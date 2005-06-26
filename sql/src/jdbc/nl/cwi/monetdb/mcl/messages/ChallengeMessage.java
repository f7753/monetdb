package nl.cwi.monetdb.mcl.messages;

import java.util.*;

/**
 * A ChallengeMessage is a server originated message, sent right after
 * the client made its connection.  In a ChallengeMessage, the server
 * identifies itself to the client, and provides a seed that the client
 * should use to salt the password when sending it back to the server.
 *
 * @author Fabian Groffen <Fabian.Groffen>
 */
public class ChallengeMessage extends MCLMessage {
	/** The character that identifies this message */
	public final char identifier = 'C';

	private final static Sentence startOfMessageSentence = new Sentence('&', "C");

	/**
	 * Constructs an empty ChallengeMessage.  The sentences need to be
	 * added using the addSentence() method.  This constructor is
	 * suitable when reconstructing messages from a stream.
	 */
	public ChallengeMessage() {
		// nothing has to be done here
		sentences = new Sentences[3];
	}

	/**
	 * Constructs a filled ChallengeMessage.  All required information
	 * is supplied and stored in this ChallengeMessage.
	 *
	 * @param protover the protocol version
	 * @param server the server identification string
	 * @throws MCLException if the server string is null
	 */
	public ChallengeMessage(int protover, String server) {
		if (server == null) throw
			new MCLException("Server identification string may not be null");
		
		sentences = new Sentences[3];
		sentences[0] = new Sentence('$', "protover\t" + protover);
		sentences[1] = new Sentence('$', "server\t" + server);
		generateSeed();
	}

	/**
	 * Returns the type of this Message as an integer type.
	 * 
	 * @return an integer value that represents the type of this Message
	 */
	public int getType() {
		return(identifier);
	}

	/**
	 * Returns the start of message sentence for this Message: &amp;C.
	 *
	 * @return the start of message sentence
	 */
	public Sentence getSomSentence() {
		return(startOfMessageSentence);
	}


	/**
	 * Adds the given String to this Message if it matches the Message
	 * type.  The sentence is parsed as far as that is considered to be
	 * necessary to validate it against the Message type.  If a sentence
	 * is not valid, an MCLException is thrown.
	 * 
	 * @param in an MCLSentence object
	 * @throws MCLException if the given sentence is not considered to
	 * be valid
	 */
	public void addSentence(MCLSentence in) throws MCLException {
		// do something with recognising the right header types, need
		// more utility functions from MCLSentence class first
	}


	/**
	 * Generates a random seed and stores it in the sentences associated
	 * with this Message.  If the sentence already exists, it is
	 * silently overwritten.
	 */
	public void generateSeed() {
		/* Perform some random stuff:
		 * - first pick a random length of the seed, between 9 and 17
		 *   characters (inclusive)
		 * - pick a random character [a-z][A-Z][0-9]
		 */
		sentences[2] = "";

		int length = 9 + Random.getInt(17 - 9 + 1);
		for (int i = 0; i < length; i++) {
			switch(Random.getInt(3)) {
				case 0:
					sentences[2] += (char)('a' + Random.getInt('z' - 'a' + 1));
				break;
				case 1:
					sentences[2] += (char)('A' + Random.getInt('Z' - 'A' + 1));
				break;
				case 2:
					sentences[2] += (char)('0' + Random.getInt('9' - '0' + 1));
				break;
			}
		}
	}
}

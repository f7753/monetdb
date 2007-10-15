/**
 * The contents of this file are subject to the MonetDB Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://monetdb.cwi.nl/Legal/MonetDBLicense-1.1.html

 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and
 * limitations under the License.

 * The Original Code is the MonetDB Database System.

 * The Initial Developer of the Original Code is CWI.
 * Portions created by CWI are Copyright (C) 1997-2007 CWI.
 * All Rights Reserved.
**/

package nl.cwi.monetdb.xquery.util;

import java.io.*;
import java.net.*;


/**
 * @author Ying Zhang <Y.Zhang@cwi.nl>
 * @version 0.1
 */

public class Extract {
	private static final int DEFAULT_BUFSIZE = 16386;

    public Extract() {}

    /* Extracts 'fromFile' from the jar package to 'toFile' */
	public static void extractFile(String fromFile, String toFile)
		throws Exception
	{
		char[] cbuf = new char[DEFAULT_BUFSIZE];
		int ret = 0;

		InputStream is = new Extract().getClass().getResourceAsStream(fromFile);

		if(is == null) {
			throw new XRPCException("File " + fromFile +
					" does not exist in the JAR package.");
		}

		BufferedReader reader = new BufferedReader(new InputStreamReader(is));
		FileWriter writer = new FileWriter(toFile, false);

		ret = reader.read(cbuf, 0, DEFAULT_BUFSIZE);
		while(ret > 0){
			writer.write(cbuf, 0, ret);
			ret = reader.read(cbuf, 0, DEFAULT_BUFSIZE);
		}
		reader.close();
		writer.close();
	}
}

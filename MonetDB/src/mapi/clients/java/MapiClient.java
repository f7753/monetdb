/*
 * The contents of this file are subject to the MonetDB Public
 * License Version 1.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at
 * http://monetdb.cwi.nl/Legal/MonetDBLicense-1.0.html
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is the Monet Database System.
 * 
 * The Initial Developer of the Original Code is CWI.
 * Portions created by CWI are Copyright (C) 1997-2003 CWI.
 * All Rights Reserved.
 * 
 * Contributor(s):
 * 		Martin Kersten <Martin.Kersten@cwi.nl>
 * 		Peter Boncz <Peter.Boncz@cwi.nl>
 * 		Niels Nes <Niels.Nes@cwi.nl>
 * 		Stefan Manegold  <Stefan.Manegold@cwi.nl>
 */

import java.io.*;
import mapi.*;

public class MapiClient 
{
    public static void usage(){
	System.out.println("Usage: java MapiClient <host> [<port> [<user> [<password> [<language>]]]]" );
	System.exit(1);
    }

    public static void main(String argv[]){
	String hostname = "localhost";
	int portnr = 50000;
	String user = "quest";
	String password = "anonymous";
	String lang = "mil";
	if (argv.length > 4 ){
	   	usage();
	}
	if (argv.length == 1){
      		user = argv[0];
	} else if (argv.length == 2){
		portnr = Integer.parseInt(argv[0]);
      		user = argv[1];
	} else if (argv.length == 3){
		hostname = argv[0];
		portnr = Integer.parseInt(argv[1]);
      		user = argv[2];
	} else if (argv.length == 4){
		hostname = argv[0];
		portnr = Integer.parseInt(argv[1]);
      		user = argv[2];
      		lang = argv[3];
	} else if (argv.length == 5){
		hostname = argv[0];
		portnr = Integer.parseInt(argv[1]);
      		user = argv[2];
      		password = argv[3];
      		lang = argv[4];
	}

	try {
      		Mapi M = new Mapi( hostname, portnr, user, password, lang );
		Reader r = new BufferedReader(new InputStreamReader(System.in));
		LineNumberReader input = new LineNumberReader(r);
		DataOutputStream out = new DataOutputStream(System.out);
		String s;
		System.out.print(M.getPrompt());
		while((s=input.readLine()) != null){
			if (s.equals("quit;")) break;
			M.quickQuery(s,out);
			System.out.print(M.getPrompt());
		}
	} catch (MapiException e){
                System.err.println( "MapiClient: "+e );
        } catch (IOException e){
                System.err.println( "MapiClient: "+e );
	}
   }
}

# The contents of this file are subject to the MonetDB Public License
# Version 1.1 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://monetdb.cwi.nl/Legal/MonetDBLicense-1.1.html
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.
#
# The Original Code is the MonetDB Database System.
#
# The Initial Developer of the Original Code is CWI.
# Portions created by CWI are Copyright (C) 1997-July 2008 CWI.
# Copyright August 2008-2009 MonetDB B.V.
# All Rights Reserved.


  require 'MonetDBConnection'
  require 'MonetDBData'
  require 'MonetDBExceptions'

   # A typical sequence of events is as follows:
    # Create a database instance (handle), invoke query using the database handle to send the statement to the server and get back a result set object.
    #
    # A result set object  has methods for fetching rows, moving around in the result set, obtaining column metadata, and releasing the result set.
    # A result set object is an instance of the MonetDBData class.
    #
    # Records can be returneds as arrays and iterators over the set.
    #
    # A database handler (dbh) is and instance of the MonetDB class.
    #
    # <b> Connection management <b/>
    #
    #  connect    -  establish a new connection
    #                * user: username (default is monetdb)
    #                * passwd: password (default is monetdb)
    #                * lang: language (default is sql) 
    #                * host: server hostanme or ip  (default is localhost)
    #                * port: server port (default is 50000)
    #                * db_name: name of the database to connect to
    #                * auth_type: hashing function to use during authentication (default is SHA1)
    #
    #  is_connected? - returns true if there is an active connection to a server, false otherwise 
    #  reconnect     - recconnect to a server
    #  close         - terminate a connection
    #  auto_commit?  - returns ture if the session is running in auto commit mode, false otherwise  
    #  auto_commit   - enable/disable auto commit mode.
    #
    #  query         - fire a query
    #
    # Currently MAPI protocols 8 and 9 are supported.
    #
    # <b> Managing record sets <b/>
    #
    #
    # A record set is represented as an instance of the MonetDBData class; the class provides methods to manage retrieved data.
    #
    #
    # The following methods allow to iterate over data:
    # 
    # fetch          - iterates over the record set and retrieves on row at a time. Each row is returned as an array.
    # fetch_hash     - iterates over columns (on cell at a time). 
    # fetch_all_hash - returns record set entries hashed by column name orderd by column position.
    # 
    # To return the record set as an array (with each tuple stored as array of fields) the following method can be used:
    # 
    # fetch_all      - fetch all rows and store them  
    #
    #
    # Information about the retrieved record set can be obtained via the following methods:
    #
    # num_rows       - returns the number of rows present in the record set
    # num_fields     - returns the number of fields (columns) that compose the schema
    # name_fields    - returns the (ordered) name of the schema's columns
    #
    # To release a record set MonetDBData#free can be used.
    #
    # <b> Type conversion </b>
    #
    # Invoking MonetDB#query with the flag type_conversion=true will result in a type cast of the record set fields from SQL types to ruby types
    #
    # demo.rb contains usage example of the above mentioned methods.

  class MonetDB
    Q_TABLE               = "1" # SELECT operation
    Q_UPDATE              = "2" # INSERT/UPDATE operations
    Q_CREATE              = "3" # CREATE/DROP TABLE operations
  
    def initalize()
      @connection = nil 
    end
  
    def connect(username = "monetdb", password = "monetdb", lang = "sql", host="127.0.0.1", port = 50000, db_name = "demo", auth_type = "SHA1")
      # TODO: handle pools of connections
      @username = username
      @password = password
      @lang = lang
      @host = host
      @port = port
      @db_name = db_name
      @auth_type = auth_type
      @connection = MonetDBConnection.new(user = @username, passwd = @password, lang = @lang, host = @host, port = @port)
      @connection.connect(@db_name, @auth_type)
    end
  
    # Send a <b> user submitted </b> query to the server and store the response
    def query(q = "", type_cast = false)
      if  @connection != nil 
        @data = MonetDBData.new(@connection, type_cast = type_cast)
        @data.execute(q)    
      end
      return @data
    end
    
    # Return true if there exists a "connection" object
    def is_connected?
      if @connection == nil
        return false
      else 
        return true
      end
    end
  
    # Reconnect to the server
    def reconnect
      if @connection != nil
        self.close
        
        @connection = MonetDBConnection.new(user = @username, passwd = @password, lang = @lang, host = @host, port = @port)
        @connection.real_connect(@db_name, @auth_type)
      end
    end
  
    # Close an active connection
    def close()
      @connection.disconnect
      @connection = nil
    end
  end
#end

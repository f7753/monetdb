# The contents of this file are subject to the MonetDB Public
# License Version 1.0 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at
# http://monetdb.cwi.nl/Legal/MonetDBLicense-1.0.html
# 
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
# 
# The Original Code is the Monet Database System.
# 
# The Initial Developer of the Original Code is CWI.
# Portions created by CWI are Copyright (C) 1997-2004 CWI.
# All Rights Reserved.
# 
# Contributor(s):
# 		Martin Kersten <Martin.Kersten@cwi.nl>
# 		Peter Boncz <Peter.Boncz@cwi.nl>
# 		Niels Nes <Niels.Nes@cwi.nl>
# 		Stefan Manegold  <Stefan.Manegold@cwi.nl>

# make rules to generate MonetDB'\''s documentation

$(prefix)/doc/www/TechDocs/Core/Mx/mxdoc.tex:	$(top_srcdir)/doc/mxdoc.tex
	-@mkdir -p $(prefix)/doc/www/TechDocs/Core/Mx
	cp $< $@

$(prefix)/doc/www/TechDocs/Core/Mx/mxdoc.aux:	$(prefix)/doc/www/TechDocs/Core/Mx/mxdoc.tex
	(cd $(prefix)/doc/www/TechDocs/Core/Mx; latex mxdoc.tex; latex mxdoc.tex)

$(prefix)/doc/www/TechDocs/Core/Mx/index.html:	$(prefix)/doc/www/TechDocs/Core/Mx/mxdoc.aux
	(cd $(prefix); latex2html -ascii_mode -no_images -address '' -style http://monetdb.cwi.nl/MonetDB.css -dir doc/www/TechDocs/Core/Mx doc/www/TechDocs/Core/Mx/mxdoc.tex)

$(prefix)/doc/www/TechDocs/APIs/Mapi/Java/Documentation/index.html:	$(top_srcdir)/src/mapi/clients/java/MapiClient.java	\
					$(top_srcdir)/src/mapi/clients/java/mapi/Mapi.java	\
					$(top_srcdir)/src/mapi/clients/java/mapi/MapiException.java
	-@mkdir -p $(prefix)/doc/www/TechDocs/APIs/Mapi/Java/Documentation
	lynx -source http://monetdb.cwi.nl/MonetDB.css > $(prefix)/doc/www/TechDocs/APIs/Mapi/Java/Documentation/MonetDB.css
	javadoc -d $(prefix)/doc/www/TechDocs/APIs/Mapi/Java/Documentation -stylesheetfile $(prefix)/doc/www/TechDocs/APIs/Mapi/Java/Documentation/MonetDB.css\
		$(top_srcdir)/src/mapi/clients/java/MapiClient.java       	\
	        $(top_srcdir)/src/mapi/clients/java/mapi/Mapi.java        	\
	        $(top_srcdir)/src/mapi/clients/java/mapi/MapiException.java

html:	$(prefix)/doc/www/TechDocs/Core/Mx/index.html	\
	$(prefix)/doc/www/TechDocs/APIs/Mapi/Java/Documentation/index.html	\
	$(top_srcdir)/doc/mkdoc.py
	mv $(prefix)/doc/www/TechDocs/Core/Mx $(prefix)/doc/
	mv $(prefix)/doc/www/TechDocs/APIs/Mapi/Java/Documentation $(prefix)/doc/
	python $(top_srcdir)/doc/mkdoc.py $(top_srcdir) $(MONET_BUILD) $(prefix)
	mkdir -p $(prefix)/doc/www/TechDocs/Core $(prefix)/doc/www/TechDocs/APIs/Mapi/Java
	mv $(prefix)/doc/Mx $(prefix)/doc/www/TechDocs/Core/
	mv $(prefix)/doc/Documentation $(prefix)/doc/www/TechDocs/APIs/Mapi/Java/


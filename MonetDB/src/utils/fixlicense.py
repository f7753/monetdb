#!/usr/bin/env python

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

# This script requires Python 2.2 or later.

# backward compatibility for Python 2.2
try:
    True
except NameError:
    False, True = 0, 1

import os, sys, getopt

usage = '''\
%(prog)s [-ar] [-l licensefile] [file...]

Options:
-a\tadd license text (default)
-r\tremove license text
-l licensefile
\tprovide alternative license text
\t(handy for removing incorrect license text)

If no file arguments, %(prog)s will read file names from standard input.

%(prog)s makes backups of all modified files.
The backup is the file with a tilde (~) appended.\
'''

license = [
    'The contents of this file are subject to the MonetDB Public',
    'License Version 1.0 (the "License"); you may not use this file',
    'except in compliance with the License. You may obtain a copy of',
    'the License at',
    'http://monetdb.cwi.nl/Legal/MonetDBLicense-1.0.html',
    '',
    'Software distributed under the License is distributed on an "AS',
    'IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or',
    'implied. See the License for the specific language governing',
    'rights and limitations under the License.',
    '',
    'The Original Code is the Monet Database System.',
    '',
    'The Initial Developer of the Original Code is CWI.',
    'Portions created by CWI are Copyright (C) 1997-2004 CWI.',
    'All Rights Reserved.',
    '',
    'Contributor(s):',
    '\t\tMartin Kersten <Martin.Kersten@cwi.nl>',
    '\t\tPeter Boncz <Peter.Boncz@cwi.nl>',
    '\t\tNiels Nes <Niels.Nes@cwi.nl>',
    '\t\tStefan Manegold  <Stefan.Manegold@cwi.nl>',
    ]

def main():
    func = addlicense
    pre = post = start = end = None
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'arl:',
                                   ['pre=', 'post=', 'start=', 'end='])
    except getopt.GetoptError:
        print >> sys.stderr, usage % {'prog': sys.argv[0]}
        sys.exit(1)
    for o, a in opts:
        if o == '-a':
            func = addlicense
        elif o == '-r':
            func = dellicense
        elif o == '-l':
            try:
                f = open(a)
            except IOError:
                print >> sys.stderr, 'Cannot open file %s' % a
                sys.exit(1)
            del license[:]
            while True:
                line = f.readline()
                if not line:
                    break
                license.append(line[:-1])
            f.close()
        elif o == '--pre':
            pre = a
        elif o == '--post':
            post = a
        elif o == '--start':
            start = a
        elif o == '--end':
            end = a

    if args:
        for a in args:
            func(a, pre=pre, post=post, start=start, end=end)
    else:
        while True:
            filename = sys.stdin.readline()
            if not filename:
                break
            func(filename[:-1], pre=pre, post=post, start=start, end=end)

suffixrules = {
    # suffix:(pre,     post,  start,  end)
    '.ag':   ('',      '',    '# ',   ''),
    '.bash': ('',      '',    '# ',   ''),
    '.c':    ('/*',    ' */', ' * ',  ''),
    '.el':   ('',      '',    '; ',   ''),
    '.h':    ('/*',    ' */', ' * ',  ''),
    '.hs':   ('',      '',    '-- ',  ''),
    '.html': ('<!--',  '-->', '',     ''),
    '.java': ('/*',    ' */', ' * ',  ''),
    '.m4':   ('',      '',    'dnl ', ''),
    '.mk':   ('',      '',    '# ',   ''),
    '.msc':  ('',      '',    '# ',   ''),
    '.mx':   ('',      '',    "@' ",  ''),
    '.php':  ('<?php', '?>',  '# ',   ''),
    '.pl':   ('',      '',    '# ',   ''),
    '.pm':   ('',      '',    '# ',   ''),
    '.py':   ('',      '',    '# ',   ''),
    '.sh':   ('',      '',    '# ',   ''),
    '.tcl':  ('',      '',    '# ',   ''),
    }

def addlicense(file, pre = None, post = None, start = None, end = None):
    if pre is None and post is None and start is None and end is None:
        root, ext = os.path.splitext(file)
        if ext == '.in':
            # special case: .in suffix doesn't count
            root, ext = os.path.splitext(root)
        if not suffixrules.has_key(ext):
            # no known suffix
            return
        pre, post, start, end = suffixrules[ext]
    if not pre:
        pre = ''
    if not post:
        post = ''
    if not start:
        start = ''
    if not end:
        end = ''
    try:
        f = open(file)
    except IOError:
        print >> sys.stderr, 'Cannot open file %s' % file
        return
    try:
        g = open(file + '.new', 'w')
    except IOError:
        print >> sys.stderr, 'Cannot create temp file %s.new' % file
        return
    line = f.readline()
    addblank = False
    if line[:2] == '#!':
        # if file starts with #! command interpreter, keep the line there
        g.write(line)
        # add a blank line
        addblank = True
        line = f.readline()
    if '-*-' in line:
        # if file starts with an Emacs mode specification, keep the line there
        g.write(line)
        # add a blank line
        addblank = True
        line = f.readline()
    if addblank:
        g.write('\n')
    if pre:
        g.write(pre + '\n')
    for l in license:
        g.write(start + l + end + '\n')
    if post:
        g.write(post + '\n')
    # add empty line after license
    g.write('\n')
    # but only one, so skip empty line from file, if any
    if line and line != '\n':
        g.write(line)
    # copy rest of file
    g.write(f.read())
    f.close()
    g.close()
    try:
        os.rename(file, file + '~')     # make backup
    except OSError:
        print >> sys.stderr, 'Cannot make backup for %s' % file
        return
    try:
        os.rename(file + '.new', file)
    except OSError:
        print >> sys.stderr, 'Cannot move file %s into position' % file

def dellicense(file, pre = None, post = None, start = None, end = None):
    if pre is None and post is None and start is None and end is None:
        root, ext = os.path.splitext(file)
        if ext == '.in':
            # special case: .in suffix doesn't count
            root, ext = os.path.splitext(root)
        if not suffixrules.has_key(ext):
            # no known suffix
            return
        pre, post, start, end = suffixrules[ext]
    if not pre:
        pre = ''
    if not post:
        post = ''
    if not start:
        start = ''
    if not end:
        end = ''
    try:
        f = open(file)
    except IOError:
        print >> sys.stderr, 'Cannot open file %s' % file
        return
    try:
        g = open(file + '.new', 'w')
    except IOError:
        print >> sys.stderr, 'Cannot create temp file %s.new' % file
        return
    line = f.readline()
    if line[:2] == '#!':
        g.write(line)
        line = f.readline()
        if line and line == '\n':
            line = f.readline()
    if '-*-' in line:
        g.write(line)
        line = f.readline()
        if line and line == '\n':
            line = f.readline()
    if pre:
        if line[:-1] == pre:
            line = f.readline()
        else:
            # doesn't match
            print >> sys.stderr, 'PRE doesn\'t match in file %s' % file
            f.close()
            g.close()
            try:
                os.unlink(file + '.new')
            except OSError:
                pass
            return
    for l in license:
        if (start + l + end).strip() == line.strip():
            line = f.readline()
        else:
            # doesn't match
            print >> sys.stderr, 'line doesn\'t match in file %s' % file
            f.close()
            g.close()
            try:
                os.unlink(file + '.new')
            except OSError:
                pass
            return
    if post:
        if line[:-1] == post:
            line = f.readline()
        else:
            # doesn't match
            print >> sys.stderr, 'POST doesn\'t match in file %s' % file
            f.close()
            g.close()
            try:
                os.unlink(file + '.new')
            except OSError:
                pass
            return
    if line and line != '\n':
        g.write(line)
    g.write(f.read())
    f.close()
    g.close()
    try:
        os.rename(file, file + '~')     # make backup
    except OSError:
        print >> sys.stderr, 'Cannot make backup for %s' % file
        return
    try:
        os.rename(file + '.new', file)
    except OSError:
        print >> sys.stderr, 'Cannot move file %s into position' % file

if __name__ == '__main__' or sys.argv[0] == __name__:
    main()

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

import sys
import os
import getopt
import string
import re

try:
    True
except NameError:
    # provide values for old Python versions
    False, True = 0, 1

class Error(Exception):
    """Bad option."""
    pass

_varprog = re.compile(r'\$\{([^}]*)\}')
_varprognt = re.compile(r'\%([^%]*)\%')

def expandvars(self, path):
    """Expand shell variables of form ${var} (and %var%).  Unknown variables
    are left unchanged."""
    i = 0
    while True:
        m = _varprog.search(path, i)
        if not m:
            break
        i, j = m.span(0)
        name = m.group(1)
        try:
            val = getattr(self, name)
        except AttributeError:
            return path
        tail = path[j:]
        path = path[:i] + val
        path += tail
    if os.name == "nt":
        i = 0
        while True:
            m = _varprognt.search(path, i)
            if not m:
                break
            i, j = m.span(0)
            name = m.group(1)
            try:
                val = getattr(self, name)
            except AttributeError:
                return path
            tail = path[j:]
            path = path[:i] + val
            path += tail
    return path

# By using a hierarchy of classes we can let Python do the searching
# for the correct value.
class BuiltinOptions:
    '''Built-in defaults.'''
    prefix = '/opt/monetdb'
    exec_prefix = '${prefix}'
    sysconfdir = '${prefix}/etc'
    gdk_dbname = 'tst'
    gdk_dbfarm = os.path.join('${prefix}/var','MonetDB')
    gdk_debug = '8'
    # gdk_mem_bigsize & gdk_vm_minsize will be set/limited to
    # 1/2 of the physically available amount of main-memory
    # during start-up in src/tools/Mserver.mx
    gdk_mem_bigsize = '256K'
    gdk_vm_minsize = '128G'
    monet_admin = 'adm'
    monet_prompt = '>'
    monet_welcome = 'yes'
    monet_mod_path = os.path.join('${exec_prefix}/lib','MonetDB') + \
                     os.pathsep + \
                     os.path.join('${exec_prefix}/lib','bin')
    monet_daemon = 'yes'

    def get(self, name, default = None):
        '''get(name, [default]) -> value

        Return the value for name after substituting ${variable}.'''
        try:
            val = getattr(self, name)
        except AttributeError:
            return default
        if type(val) is type(''): # only do this on string args
            return expandvars(self, val);
        return val

class SystemOptions(BuiltinOptions):
    '''Values from the system config file.'''
    pass

class ConfigOptions(SystemOptions):
    '''Values from a user-supplied config file.'''
    pass

# These are lists of command-line options.  For each option, we can
# specify a short and a long form, and whether the option takes an
# argument.  Also, if the GDK option name is given, the option will
# automatically be recorded (this should always be done for options
# passed to parse_options).
# The argument field is a descriptive string for the argument.  If
# None is given, the option does not take an argument.
# The comment is a very brief description of what the option is for.
# default_options are always available, cmd_options are the extra
# options that are recognized by default.  These latter can be
# overridden by the call to parse_options.
default_options = [
    # long name, short name, GDK option, argument, comment
    ('config','c',None,'config_file', 'read options from the given file'),
    ('help','?',None,None,'print usage message'),
    ('set','s',None,'option=value', 'set a named option'),
    ]
cmd_options = [
    ('debug','d','gdk_debug','debug_level', None),
    ('dbname',None,'gdk_dbname','database_name', None),
    ('dbfarm',None,'gdk_dbfarm','database_directory', None),
    ]

def usage(cmd_options, cmd = None):
    '''usage(options)

    Print usage information.'''
    if cmd is None:
        cmd = '%s [ options ]' % os.path.basename(sys.argv[0])
    sys.stderr.write('Usage: %s\n' % cmd)
    sys.stderr.write('Options are:\n')
    for long, short, gdk_opt, argument, comment in default_options + cmd_options:
        arg = []
        if short:
            arg.append('-' + short)
        if long:
            arg.append('--' + long)
        arg = string.join(arg, ' or ')
        if argument:
            arg = arg + ' ' + argument
        if comment:
            arg = arg + '\n\t' + string.replace(comment, '\n', '\n\t')
        sys.stderr.write(arg+'\n')
    raise Error('usage')

def parse_config(file, options = ConfigOptions):
    '''parse_config(file)

    Parse a Monet config file.'''
    try:
        f = open(file)
    except IOError:
        sys.stderr.write('Could not open file %s\n' % file)
        return
    while True:
        line = f.readline()
        if not line:
            # end of file
            break
        line = string.strip(line)
        if not line or line[0] == '#':
            # ignore comments and empty lines
            continue
        keyval = string.split(line, '=', 1)
        if len(keyval) != 2:
            sys.stderr.write('syntax error in %s\n' % file)
            f.close()
            sys.exit(1)
        key, val = keyval
        key = string.strip(key)
        val = string.strip(val)
        if not key:
            sys.stderr.write('syntax error in %s\n' % file)
            f.close()
            sys.exit(1)
        quote = False
        for i in range(len(val)):
            c = val[i]
            if c == '"':
                quote = not quote
            elif not quote and c == '#':
                val = string.strip(val[:i])
                break
        if quote:
            sys.stderr.write('syntax error in %s\n' % file)
            f.close()
            sys.exit(1)
        val = string.replace(val, '"', '')
        setattr(options, key, val)

def print_options(options):
    '''print_options(options)

    Print the current values of the options.'''
    for name in dir(options):
        if name[:1] == '_':
            continue
        val = getattr(options, name)
        sys.stderr.write('%s = %s\n' % (name, val))

def parse_options(argv, cmd_options = cmd_options, usage = usage, default_config = True):
    '''parse_options(argv) -> options

    This is the main interface to this module.

    Parse the command line options (argv = sys.argv[1:]) and return a
    class instance describing the options settings.'''
    all_options = default_options + cmd_options
    debug = 0
    options = ConfigOptions()
    seen_config = False
    getopt_short = ''
    getopt_long = []
    for long, short, gdk_opt, argument, comment in all_options:
        if short:
            getopt_short = getopt_short + short
            if argument:
                getopt_short = getopt_short + ':'
        if long:
            if argument:
                q = '='
            else:
                q = ''
            getopt_long.append(long + q)
    opts, args = getopt.getopt(argv, getopt_short, getopt_long)
    for o, a in opts:
        if o[:2] == '--':
            o = o[2:]
            for long, short, gdk_opt, argument, comment in all_options:
                if o == long:
                    break
            else:
                usage(cmd_options)
        else:
            o = o[1:]
            for long, short, gdk_opt, argument, comment in all_options:
                if o == short:
                    break
            else:
                usage(cmd_options)
        if long == 'config':
            options.config = a
            parse_config(a, ConfigOptions)
            seen_config = True
        if gdk_opt:
            if not argument:
                a = True
            setattr(options, gdk_opt, a)
            if long == 'debug':
                debug = 1
        elif long == 'set':
            keyval = string.split(a, '=', 1)
            if len(keyval) != 2:
                sys.stderr.write('wrong format for --set option %s\n' % a)
            else:
                key, val = keyval
                key = string.strip(key)
                val = string.strip(val)
                if not key:
                    sys.stderr.write('wrong format for --set option %s\n' % a)
                else:
                    setattr(options, key, val)
        elif long == 'help':
            usage(cmd_options)
    if default_config and not seen_config:
        options.config = os.path.join(BuiltinOptions().get('sysconfdir'),'MonetDB.conf')
        parse_config(options.config, SystemOptions)
    if debug:
        print_options(options)
    return options, args

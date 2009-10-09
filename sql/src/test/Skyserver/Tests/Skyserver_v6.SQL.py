import os, sys
try:
    import subprocess
except ImportError:
    # use private copy for old Python versions
    import MonetDBtesting.subprocess26 as subprocess

def main():
    dir = os.getenv('TSTSRCDIR')
    clcmd = str(os.getenv('SQL_CLIENT'))
    clcmd1 = str(os.getenv('SQL_CLIENT')) + " -uskyserver -Pskyserver"
    sys.stdout.write('Create User\n')
    clt = subprocess.Popen(clcmd + ' "%s"' % os.path.join(dir, '..','create_user.sql'), shell = True, universal_newlines = True, stdout = subprocess.PIPE)
    out,err = clt.communicate()
    sys.stdout.write(out)
    sys.stdout.write('tables\n')
    clt1 = subprocess.Popen(clcmd1 + ' "%s"' % os.path.join(dir, '..','..','..','sql','math.sql'), shell = True, universal_newlines = True, stdout = subprocess.PIPE)
    out,err = clt1.communicate()
    sys.stdout.write(out)
    clt1 = subprocess.Popen(clcmd1 + ' "%s"' % os.path.join(dir, '..','..','..','sql','cache.sql'), shell = True, universal_newlines = True, stdout = subprocess.PIPE)
    out,err = clt1.communicate()
    sys.stdout.write(out)
    clt1 = subprocess.Popen(clcmd1 + ' "%s"' % os.path.join(dir, '..','..','..','sql','skyserver.sql'), shell = True, universal_newlines = True, stdout = subprocess.PIPE)
    out,err = clt1.communicate()
    sys.stdout.write(out)
    clt1 = subprocess.Popen(clcmd1 + ' "%s"' % os.path.join(dir, '..','Skyserver_tables_v6.sql'), shell = True, universal_newlines = True, stdout = subprocess.PIPE)
    out,err = clt1.communicate()
    sys.stdout.write(out)
    clt1 = subprocess.Popen(clcmd1, shell = True, universal_newlines = True, stdin = subprocess.PIPE, stdout = subprocess.PIPE)
    sql = open(os.path.join(dir, '..', 'Skyserver_import_v6.sql')).read().replace('DATA',os.path.join(dir,'..','microsky_v6').replace('\\','\\\\'))
    out,err = clt1.communicate(sql)
    sys.stdout.write(out)
    clt1 = subprocess.Popen(clcmd1 + ' "%s"' % os.path.join(dir, '..','Skyserver_constraints_v6.sql'), shell = True, universal_newlines = True, stdout = subprocess.PIPE)
    out,err = clt1.communicate()
    sys.stdout.write(out)
    sys.stdout.write('views\n')
    clt1 = subprocess.Popen(clcmd1 + ' "%s"' % os.path.join(dir, '..','Skyserver_views_v6.sql'), shell = True, universal_newlines = True, stdout = subprocess.PIPE)
    out,err = clt1.communicate()
    sys.stdout.write(out)
    sys.stdout.write('functions\n')
    clt1 = subprocess.Popen(clcmd1 + ' "%s"' % os.path.join(dir, '..','Skyserver_functions_v6.sql'), shell = True, universal_newlines = True, stdout = subprocess.PIPE)
    out,err = clt1.communicate()
    sys.stdout.write(out)
    sys.stdout.write('Cleanup\n')
    clt1 = subprocess.Popen(clcmd1 + ' "%s"' % os.path.join(dir, '..','Skyserver_dropFunctions_v6.sql'), shell = True, universal_newlines = True, stdout = subprocess.PIPE)
    out,err = clt1.communicate()
    sys.stdout.write(out)
    clt1 = subprocess.Popen(clcmd1 + ' "%s"' % os.path.join(dir, '..','Skyserver_dropMs_functions.sql'), shell = True, universal_newlines = True, stdout = subprocess.PIPE)
    out,err = clt1.communicate()
    sys.stdout.write(out)
    clt1 = subprocess.Popen(clcmd1 + ' "%s"' % os.path.join(dir, '..','Skyserver_dropMath.sql'), shell = True, universal_newlines = True, stdout = subprocess.PIPE)
    out,err = clt1.communicate()
    sys.stdout.write(out)
    clt1 = subprocess.Popen(clcmd1 + ' "%s"' % os.path.join(dir, '..','Skyserver_dropCache.sql'), shell = True, universal_newlines = True, stdout = subprocess.PIPE)
    out,err = clt1.communicate()
    sys.stdout.write(out)
    clt1 = subprocess.Popen(clcmd1 + ' "%s"' % os.path.join(dir, '..','Skyserver_dropViews_v6.sql'), shell = True, universal_newlines = True, stdout = subprocess.PIPE)
    out,err = clt1.communicate()
    sys.stdout.write(out)
    clt1 = subprocess.Popen(clcmd1 + ' "%s"' % os.path.join(dir, '..','Skyserver_dropConstraints_v6.sql'), shell = True, universal_newlines = True, stdout = subprocess.PIPE)
    out,err = clt1.communicate()
    sys.stdout.write(out)
    clt1 = subprocess.Popen(clcmd1 + ' "%s"' % os.path.join(dir, '..','Skyserver_dropTables_v6.sql'), shell = True, universal_newlines = True, stdout = subprocess.PIPE)
    out,err = clt1.communicate()
    sys.stdout.write(out)
    sys.stdout.write('Remove User\n')
    clt = subprocess.Popen(clcmd + ' "%s"' % os.path.join(dir, '..','drop_user.sql'), shell = True, universal_newlines = True, stdout = subprocess.PIPE)
    out,err = clt.communicate()
    sys.stdout.write(out)

main()

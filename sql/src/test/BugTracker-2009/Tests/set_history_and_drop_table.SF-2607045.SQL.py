import os, sys
import subprocess

def main():
    dir = os.getenv('TSTSRCDIR')
    clcmd = str(os.getenv('SQL_CLIENT'))
    sys.stdout.write('Load history\n')
    clt1 = subprocess.Popen(clcmd + ' "%s"' % os.path.join(dir,'..', '..','..','sql','history.sql'), shell = True, universal_newlines = True, stdout = subprocess.PIPE)
    out, err = clt1.communicate()
    sys.stdout.write(out)
    sys.stdout.write('Run test\n')
    clt1 = subprocess.Popen(clcmd + ' "%s"' % os.path.join(dir,'..','set_history_and_drop_table.SF-2607045.sql'), shell = True, universal_newlines = True, stdout = subprocess.PIPE)
    out, err = clt1.communicate()
    sys.stdout.write(out)
    sys.stdout.write('Drop history\n')
    clt1 = subprocess.Popen(clcmd + ' "%s"' % os.path.join(dir,'..','drop_history.sql'), shell = True, universal_newlines = True, stdout = subprocess.PIPE)
    out, err = clt1.communicate()
    sys.stdout.write(out)

main()

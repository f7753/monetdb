import os, sys
import copy
import subprocess


def client(cmd, env = os.environ):
    clt = subprocess.Popen(cmd, env=env, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    sys.stdout.write(clt.stdout.read())
    clt.stdout.close()
    sys.stderr.write(clt.stderr.read())
    clt.stderr.close()



def main():
    testenv = copy.deepcopy(os.environ)
    testenv['DOTMONETDBFILE'] = '.testuser'
    f = open(testenv['DOTMONETDBFILE'], 'w')
    f.write('user=my_user\npassword=p1\n')
    f.close()
    clcmd = str(os.getenv('SQL_CLIENT')) + "< %s" % ('%s/../table.sql' % os.getenv('RELSRCDIR'))
    client(clcmd)
    os.unlink(testenv['DOTMONETDBFILE'])

main()

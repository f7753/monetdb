import os, sys
import subprocess

def client(cmd):
    clt = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    sys.stdout.write(clt.stdout.read())
    clt.stdout.close()
    sys.stderr.write(clt.stderr.read())
    clt.stderr.close()


def main():
    clcmd = str(os.getenv('SQL_CLIENT')) + " -umy_user2 -Pp2 < %s" % ('%s/../role.sql' % os.getenv('RELSRCDIR'))
    client(clcmd)

main()

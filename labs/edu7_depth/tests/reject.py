import subprocess,sys
p=subprocess.run(sys.argv[1:],text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
print(p.stdout,end='')
sys.exit(0 if p.returncode==1 and 'E7_CHECK:' in p.stdout else 1)

import sys
import os

try:
    file = open(".cgi_pid.pid", "r")
    line = file.readline()
    print("line", line)
    os.system("kill " + line)
except IOError:
    print("IOError")

cmd = "spawn-fcgi -f /Users/gloomypan/Projects/cpp/convert_cgi/cmake-build-debug/convert_cgi -a 127.0.0.1 -p 9000"
val = os.popen(cmd)
retStr: str = ''
for line in val.readlines():
    retStr += line

print("======================================================================")
print(retStr)

pid = retStr.split("ID: ")[1]
print("pid", pid)

file = open(".cgi_pid.pid", "w+")

file.write(pid)

file.close()

import subprocess
import os
import hashlib

def md5(fname):
    hash = hashlib.md5()
    with open(fname, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash.update(chunk)
    return hash.hexdigest()

for filename in os.listdir('raw'):
     if(filename[0] != '.'):
     	rawhash = md5('raw/' + filename)
     	decodedhash = md5('decoded/' + filename)
     	print filename + ' = rawhash:' + rawhash + " resulthash:" + decodedhash + " = " + str(rawhash == decodedhash)

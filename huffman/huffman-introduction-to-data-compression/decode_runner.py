import subprocess
import os
for filename in os.listdir('raw'):
     if(filename[0] != '.'):
     	print filename
     	cmd = "./huff_dec -i result/" + filename + " -o decoded/" + filename
     	os.system(cmd)

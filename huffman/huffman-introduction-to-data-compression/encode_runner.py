import subprocess
import os
for filename in os.listdir('raw'):
     if(filename[0] != '.'):
     	print filename
     	cmd = "./huff_enc -i raw/" + filename + " -o result/" + filename
     	os.system(cmd)

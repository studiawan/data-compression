import subprocess
import os
import hashlib

print "FILE\tresultsize\trawsize\tpercent"
for filename in os.listdir('raw'):
	if(filename[0] != '.'):
		rawsize = os.stat('raw/' + filename).st_size
		resultsize = os.stat('result/' + filename).st_size
		result = resultsize * 100.0 / rawsize
		print filename + "\t" + str(resultsize) + "\t" + str(rawsize) + "\t" + str(result)

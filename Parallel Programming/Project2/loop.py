import os
for s in [ 64,132,256,512,1024,2096,4096 ,8196]:
	for t in [ 1,2,4,8,16,32,64]:
		cmd = "g++ -DNUMT=%d -DNUMNODES=%d project2.cpp -o prog -lm -fopenmp" % ( t,s )
		os.system( cmd )
		cmd = "./prog"
		os.system( cmd )
# A typical Rakefile

PORT  = 6285
HOSTS = 127.0.0.1 
#192.168.0.95

actionset1:
	echo starting actionset1
	remote-cat external_file
		requires square.c aaa.txt

actionset2:
	echo starting actionset2
	remote-cal -m 4
		requires aaaa.txt

#actionset3:
#	echo starting actionset3
#	ls
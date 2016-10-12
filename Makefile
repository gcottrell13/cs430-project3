all:
	gcc -o a project.c -lm 

test1:
	./a 20 20 good01.json output20x20.ppm
test2:
	./a 5 5 good01.json output5x5.ppm
test3:
	./a 10 10 good01.json output10x10.ppm
testlarge:
	./a 1000 1000 good01.json output_large.ppm

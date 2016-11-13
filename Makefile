all:
	gcc -o raycast project.c -lm 

test1:
	./raycast 20 20 good01.json output20x20.ppm
test2:
	./raycast 5 5 good01.json output5x5.ppm
test3:
	./raycast 100 100 project_descript.json output100x100.ppm
testlarge:
	./raycast 1000 1000 good01.json output_large.ppm

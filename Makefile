build:
	gcc process_generator.c -g -o process_generator.out -lm
	gcc clk.c -g  -o clk.out -lm
	gcc scheduler.c -g -o scheduler.out -lm
	gcc process.c -g -o process.out -lm
	gcc test_generator.c  -g -o test_generator.out -lm

clean:
	rm -f *.out *.h.gch

all: clean build run

run:
	./process_generator.out processes.txt -sch 3 -q 2 -mem 1

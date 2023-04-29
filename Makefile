build:
	gcc process_generator.c -o process_generator.out -lm
	gcc clk.c  -o clk.out -lm
	gcc scheduler.c  -o scheduler.out -lm
	gcc process.c  -o process.out -lm
	gcc test_generator.c   -o test_generator.out -lm

clean:
	rm -f *.out *.h.gch

all: clean build run

run:
	./process_generator.out  processes.txt -sch 3 -q 1 -mem 1

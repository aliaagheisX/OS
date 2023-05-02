build:
	gcc process_generator.c -O3 -w -o process_generator.out -lm
	gcc clk.c -O3 -w -o clk.out -lm
	gcc scheduler.c -w -o scheduler.out -lm
	gcc process.c -w -o process.out -lm

clean:
	rm -f *.out *.h.gch

all: clean build run

run:
	./process_generator.out processes.txt -sch 2 - -mem 2

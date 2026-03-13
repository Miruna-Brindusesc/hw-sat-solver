TARGET = sat_solver
CC = gcc
CFLAGS = -Wall -Wextra -O2

# Default build target
build: $(TARGET)

# Compile the SAT solver
$(TARGET): sat_solver.c
	$(CC) $(CFLAGS) -o $(TARGET) sat_solver.c

# Run target: execute the SAT solver with specified input and output
run: $(TARGET)
	@echo "Running the SAT solver..."
	@echo "Input file: $(INPUT)"
	@echo "Output file: $(OUTPUT)"
	./$(TARGET) $(INPUT) $(OUTPUT)

# Clean target: remove the binary and any temporary files
clean:
	@echo "Cleaning up..."
	rm -f $(TARGET) *.tmp
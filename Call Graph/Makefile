# Variables
CPP_FILE=input.cpp       # Source C++ file
OUTPUT_GRAPH=callgraph.txt # Output text file for function call graph
CLANG=clang                # Clang compiler

# Default target
all: generate_call_graph

# Generate the function call graph
generate_call_graph: $(CPP_FILE)
	$(CLANG) -cc1 -analyze -analyzer-checker=debug.DumpCallGraph $(CPP_FILE) 2>&1 | tee $(OUTPUT_GRAPH)

# Clean up
clean:
	@rm -f $(OUTPUT_GRAPH)

# Rebuild
rebuild: clean all
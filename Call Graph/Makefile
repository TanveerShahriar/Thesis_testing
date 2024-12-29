# Variables
CPP_FILE=input.cpp           # Source C++ file
RAW_OUTPUT=raw_callgraph.txt # Temporary raw output file
OUTPUT_GRAPH=callgraph.json  # Final structured output file
CLANG=clang                  # Clang compiler
PROCESS_SCRIPT=process_callgraph.py # Script to format output

# Default target
all: generate_call_graph

# Generate the raw function call graph and process it
generate_call_graph: $(CPP_FILE)
	$(CLANG) -cc1 -analyze -analyzer-checker=debug.DumpCallGraph $(CPP_FILE) > $(RAW_OUTPUT) 2>&1
	python $(PROCESS_SCRIPT) $(RAW_OUTPUT) $(OUTPUT_GRAPH)
	@rm -f $(RAW_OUTPUT)

# Clean up
clean:
	@rm -f $(RAW_OUTPUT) $(OUTPUT_GRAPH)

# Rebuild
rebuild: clean all
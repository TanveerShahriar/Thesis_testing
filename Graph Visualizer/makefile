# Define the Python file and the file to clean
PYTHON_FILE = graph_v.py
CALL_GRAPH_DIR = ../Call\ Graph

# Default target to run the Python file
.PHONY: run
run:
	make -C $(CALL_GRAPH_DIR)
	python $(PYTHON_FILE)

.PHONY: clean
clean:
	make -C $(CALL_GRAPH_DIR) clean
# Define the Python file and the file to clean
PYTHON_FILE = graph_v.py
JSON_FILE = adjacency_list.json

# Default target to run the Python file
.PHONY: run
run:
	python $(PYTHON_FILE)

# Clean target to remove the adjacency_list.json file
.PHONY: clean
clean:
	rm -f $(JSON_FILE)

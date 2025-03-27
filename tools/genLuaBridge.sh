#!/bin/bash

# Set paths
JUCE_PROJECT_PATH="/path/to/your/juce/project"
DOXYGEN_CONFIG_PATH="/path/to/your/Doxyfile"
OUTPUT_DIR="${JUCE_PROJECT_PATH}/generated_bindings"
DOXYGEN_XML_PATH="${OUTPUT_DIR}/xml"

# Ensure the output directory exists
mkdir -p "$OUTPUT_DIR"

# Step 1: Run Doxygen to generate the XML documentation
echo "Running Doxygen..."
doxygen "$DOXYGEN_CONFIG_PATH"

# Step 2: Check if the Doxygen XML directory exists
if [ ! -d "$DOXYGEN_XML_PATH" ]; then
  echo "Error: Doxygen XML output not found at $DOXYGEN_XML_PATH"
  exit 1
fi

# Step 3: Run the Python script to generate LuaBridge bindings
echo "Generating LuaBridge bindings from Doxygen XML..."

python3 generate_luabridge_bindings.py "$DOXYGEN_XML_PATH"

# Step 4: Move the generated LuaBridge binding file into the project
echo "Moving generated LuaBridge bindings to the JUCE project..."

# Ensure the output file name is unique
OUTPUT_CPP="${OUTPUT_DIR}/generated_luabridge_bindings.cpp"
if [ -f "$OUTPUT_CPP" ]; then
  mv "$OUTPUT_CPP" "${OUTPUT_DIR}/generated_luabridge_bindings_$(date +%Y%m%d%H%M%S).cpp"
fi

echo "LuaBridge bindings successfully generated and saved to $OUTPUT_CPP"


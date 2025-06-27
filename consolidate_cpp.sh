#!/bin/bash

# Script to consolidate all .cpp and .h files in a directory tree into a single text file
# Usage: ./consolidate_cpp.sh [directory] [output_file]

# Set default values
SOURCE_DIR="${1:-.}"  # Use current directory if no argument provided
OUTPUT_FILE="${2:-cpp_project_consolidated.txt}"  # Default output filename

# Check if source directory exists
if [ ! -d "$SOURCE_DIR" ]; then
    echo "Error: Directory '$SOURCE_DIR' does not exist."
    exit 1
fi

# Clear the output file if it exists
> "$OUTPUT_FILE"

echo "Consolidating C++ project files from: $SOURCE_DIR"
echo "Output file: $OUTPUT_FILE"
echo "================================================="

# Counter for files processed
file_count=0

# Find all .cpp and .h files recursively and process them
find "$SOURCE_DIR" -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.cc" -o -name "*.cxx" \) | sort | while read -r file; do
    # Get relative path from source directory
    relative_path=$(realpath --relative-to="$SOURCE_DIR" "$file")
    
    echo "Processing: $relative_path"
    
    # Add file header to output
    echo "// ==================== $relative_path ====================" >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"
    
    # Add file contents
    cat "$file" >> "$OUTPUT_FILE"
    
    # Add separator
    echo "" >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"
    
    ((file_count++))
done

echo "================================================="
echo "Consolidation complete!"
echo "Files processed: $file_count"
echo "Output saved to: $OUTPUT_FILE"

# Show file size
if [ -f "$OUTPUT_FILE" ]; then
    file_size=$(du -h "$OUTPUT_FILE" | cut -f1)
    echo "Output file size: $file_size"
fi

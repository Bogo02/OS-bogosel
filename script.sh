#!/bin/bash

# Check if the script is called with exactly one argument
if (( $# != 1 )); then
    echo "Usage: $0 <filename.c>"
    exit 1
fi

# Assign the argument to a variable and extract the filename and extension
filename="$1"
name="${filename%.*}"
extension="${filename##*.}"

# Check if the file exists
if [[ ! -f "$filename" ]]; then
    echo "File not found: $filename"
    exit 1
fi

# Check if the file has a .c extension
if [[ "$extension" != "c" ]]; then
    echo "Invalid file extension: $extension"
    exit 1
fi

# Compile the file and capture the output
output=$(gcc -o "$name" -Wall -Wextra "$filename" 2>&1)

# Count the number of errors and warnings
errors=$(grep -c "error:" <<< "$output")
warnings=$(grep -c "warning:" <<< "$output")

# Print the results
echo "Errors: $errors"
echo "Warnings: $warnings"
#did not manage to test if the script.sh file works properly, my laptop saved the file who knows where, sorry

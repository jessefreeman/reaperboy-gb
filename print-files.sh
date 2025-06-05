#!/bin/bash

# print-files.sh
#
# Description:
#   Dumps the contents of one or more files or directories into a single text file for inspection,
#   sharing, or use as LLM context. Each file is prefixed with a clear header.
#   Default output file: .print_output.tmp.txt (auto-added to .gitignore).
#
# Installation:
#   1. Save this file as print-files.sh in your project root.
#   2. Make it executable:
#        chmod +x print-files.sh
#   3. (Optional) Move it to a directory in your PATH or create an alias for easy access.
#
# Usage Examples:
#   ./print-files.sh src/utils/helpers.ts
#     → Outputs just that file’s content.
#
#   ./print-files.sh src/components
#     → Outputs all files in the src/components directory (non-recursive).
#
#   ./print-files.sh src/utils src/hooks
#     → Outputs all files in both directories.
#
#   ./print-files.sh src/index.ts > my-output.txt
#     → Outputs to a custom file instead of the default.

#!/bin/bash

TMP_FILE=".print_output.tmp.txt"

if ! grep -qxF "$TMP_FILE" .gitignore 2>/dev/null; then
  echo "$TMP_FILE" >> .gitignore
fi

: > "$TMP_FILE"
echo "Writing to: $TMP_FILE"

print_file() {
  echo "### FILE: $1" >> "$TMP_FILE"
  cat "$1" >> "$TMP_FILE"
  echo -e "\n\n" >> "$TMP_FILE"
  echo "✔ Added: $1"
}

for path in "$@"
do
  if [ -d "$path" ]; then
    for file in "$path"/*; do
      [ -f "$file" ] && print_file "$file"
    done
  elif [ -f "$path" ]; then
    print_file "$path"
  else
    echo "⚠️ Skipped invalid path: $path"
  fi
done

echo "Done. Output saved to $TMP_FILE"

# Termination in failure.
set -euo pipefail

# Setting up the locations.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SRC_DIR="$PROJECT_ROOT/compiler/src/fe"
TARGET_DIR="$PROJECT_ROOT/tests"

# Accept the filename from the user or prompt if none provided.
if [ "$#" -gt 0 ]; then
  PARAMS=("$@")
else
  read -rp "Enter parameter to pass to main: " USER_PARAM
  PARAMS=("$USER_PARAM")
fi

# Resolve file paths for any parameters that look like files.
# If a parameter doesn't exist relative to the current working directory,
# Try to locate it under the project root (for example, $PROJECT_ROOT/tests/<name>).
for i in "${!PARAMS[@]}"; do
  p="${PARAMS[i]}"

  # If the path exists as given, convert to absolute path.
  if [ -e "$p" ]; then
    PARAMS[i]="$(cd "$(dirname "$p")" && pwd)/$(basename "$p")"
    continue
  fi

  # Strip leading ./ for basename checks.
  base="${p#./}"

  if [ -e "$PROJECT_ROOT/$p" ]; then
    PARAMS[i]="$PROJECT_ROOT/$p"
    continue
  fi

  if [ -e "$PROJECT_ROOT/tests/$base" ]; then
    PARAMS[i]="$PROJECT_ROOT/tests/$base"
    continue
  fi

  if [ -e "$PROJECT_ROOT/$base" ]; then
    PARAMS[i]="$PROJECT_ROOT/$base"
    continue
  fi

  # Otherwise leave parameter as-is (it may be a flag or intended relative path).
done

# Ensure source directory exists
if [ ! -d "$SRC_DIR" ]; then
  echo "ERROR: Source directory '$SRC_DIR' not found." >&2
  exit 1
fi

cd "$SRC_DIR"

echo "Compiling in $SRC_DIR..."
if ! make; then
  echo "ERROR: Make failed." >&2
  exit 1
fi

if [ ! -f main ]; then
  echo "ERROR: Compiled 'main' not found in $SRC_DIR." >&2
  exit 1
fi

# Ensure target directory exists.
if [ ! -d "$TARGET_DIR" ]; then
  echo "Target directory '$TARGET_DIR' does not exist. Creating it..."
  mkdir -p "$TARGET_DIR"
fi

echo "Moving 'main' to $TARGET_DIR/..."
mv -f main "$TARGET_DIR/"
chmod +x "$TARGET_DIR/main"

echo "Cleaning build artifacts in $SRC_DIR..."
if ! make clean; then
  echo "ERROR: make clean failed." >&2
  exit 1
fi

echo "Running $TARGET_DIR/main with parameters: ${PARAMS[*]}..."
"$TARGET_DIR/main" "${PARAMS[@]}"

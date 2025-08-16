# Apply a regex to the input file SOURCE, and write the result to TARGET.
#
# The regex searches for lines that look like
#
#     #line <number> "path"
#
# and replaces them with absolute paths
#
#     #line <number> "SOURCE_DIR/path"

file(READ "${SOURCE}" TEXT)
string(REGEX REPLACE "#line ([0-9]*) \"([^\"]*)\"" "#line \\1 \"${SOURCE_DIR}/\\2\"" TEXT "${TEXT}")
file(WRITE "${TARGET}" "${TEXT}")

## Contribution Guidelines

We conform to [Google C++ Style Guidelines][google].

In particular:
- For C/C++, use `.c`/`.cc` and `.h` for source files and header files;
- Each line of code should not exceed 80 characters;
- Source files are organised by the functionality they implement, so a directory
  includes both the header and source files;
- All code must be contained in a namespace that matches their target;
- No importing or aliasing (`using`) of namespace at all;
- When it comes to integer types, prefer `uint64_t`, `uint32_t`, `uint16_t`,
  `uint8_t`;
- For indentation, each tab is strictly 2 characters and should be "expanded" to
  become spaces;
- `CamelCase` for class and struct names;
- `snake_case` for functions, methods, and local/member variables;
- `kConstant` or `UPPER_SNAKE_CASE` for constants.

Use "clang-format --style=Google" to make sure your code conforms to them.

[google]: (https://google.github.io/styleguide/cppguide.html)

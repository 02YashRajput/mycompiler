# MyCompiler

A simple compiler written in C++ that translates a custom programming language into x86-64 assembly code. This compiler demonstrates the fundamental phases of compilation: lexical analysis, parsing, and code generation.

## Features

- **Custom Programming Language**: Simple syntax supporting constants, arithmetic operations, comparisons, and control flow
- **Multi-phase Compilation**: Clean separation of tokenization, parsing, and code generation
- **x86-64 Assembly Output**: Generates native assembly code for Linux x86-64 systems
- **Expression Evaluation**: Support for complex arithmetic and comparison expressions
- **Variable Scoping**: Block-scoped constant variables with shadowing support
- **Memory Management**: Custom arena allocator for efficient AST node allocation

## Language Syntax

### Data Types

- **Integers**: Literal integer values (e.g., `42`, `100`)

### Variables

- **Constants**: Immutable variables declared with `const`

```
const x = 10;
const y = x + 5;
```

### Expressions

- **Arithmetic Operations**: `+`, `-`, `*`, `/`, `%`
- **Comparison Operations**: `==`, `!=`, `<`, `>`, `<=`, `>=`
- **Parentheses**: For grouping expressions `(expression)`

### Statements

- **Constant Declaration**: `const identifier = expression;`
- **Exit Statement**: `exit expression;` (terminates program with expression value as exit code)
- **Block Scope**: `{ statements... }` for scoped variable declarations

### Example Program

```
const x = 10;
const y = 20;
{
    const z = x + y;
    exit z > 25;
}
```

## Prerequisites

- **C++ Compiler**: Supporting C++20 standard (GCC 10+ or Clang 10+)
- **CMake**: Version 3.20 or higher
- **NASM**: Netwide Assembler for x86-64
- **GNU Binutils**: For linking (`ld`)
- **Linux x86-64**: Currently targets Linux systems

### Installing Prerequisites

**Ubuntu/Debian:**

```bash
sudo apt update
sudo apt install build-essential cmake nasm binutils
```

**Arch Linux:**

```bash
sudo pacman -S base-devel cmake nasm binutils
```

**Fedora:**

```bash
sudo dnf install gcc-c++ cmake nasm binutils
```

## Building

1. **Clone the repository:**

```bash
git clone https://github.com/02YashRajput/mycompiler.git
cd mycompiler
```

2. **Create build directory:**

```bash
mkdir -p build
cd build
```

3. **Configure and build:**

```bash
cmake ..
cmake --build .
```

## Usage

### Basic Usage

1. **Create a source file** (e.g., `program.txt`):

```
const result = 15 + 10;
exit result;
```

2. **Compile and run:**

```bash
./build/mycompiler program.txt
./out
echo $?  # Shows the exit code
```

### Using the Convenience Script

The project includes a `run.sh` script that automates the build and execution process:

```bash
chmod +x run.sh
./run.sh
```

This script will:

- Build the compiler using CMake
- Compile the `input.txt` file
- Execute the resulting binary
- Display the exit code

### Step-by-Step Process

The compiler follows these phases:

1. **Tokenization**: Converts source code into tokens
2. **Parsing**: Builds an Abstract Syntax Tree (AST)
3. **Code Generation**: Produces x86-64 assembly code
4. **Assembly**: Uses NASM to create object files
5. **Linking**: Uses LD to create executable

## Project Structure

```
mycompiler/
├── src/
│   ├── main.cpp           # Main driver program
│   ├── tokenization.hpp   # Lexical analyzer
│   ├── parser.hpp         # Parser and AST definitions
│   ├── generator.hpp      # x86-64 code generator
│   └── arenaAllocator.hpp # Memory allocator for AST nodes
├── CMakeLists.txt         # Build configuration
├── Dockerfile             # Container build setup
├── docker-compose.yaml    # Container orchestration
├── flake.nix             # Nix development environment
├── run.sh                # Convenience build/run script
├── input.txt             # Example source code
└── README.md             # This file
```

## Architecture

### Tokenizer (`tokenization.hpp`)

- Converts source code into a stream of tokens
- Handles keywords, operators, identifiers, and literals
- Performs basic syntax validation

### Parser (`parser.hpp`)

- Implements recursive descent parsing
- Builds Abstract Syntax Tree (AST)
- Handles operator precedence and associativity
- Uses arena allocator for memory management

### Code Generator (`generator.hpp`)

- Traverses AST and generates x86-64 assembly
- Manages register allocation and stack operations
- Implements variable scoping and symbol tables
- Handles system calls for program termination

## Development

### Docker Support

Build and run using Docker:

```bash
# Build the container
docker-compose build

# Run the compiler
docker-compose run compiler
```

### Nix Support

For reproducible development environment:

```bash
nix develop  # Enter development shell
```

## Examples

### Simple Arithmetic

```
const a = 10;
const b = 20;
const sum = a + b;
exit sum;
```

### Comparison Operations

```
const x = 15;
const y = 10;
exit x > y;  # Exits with code 1 (true)
```

### Scoped Variables

```
const global = 100;
{
    const local = 50;
    const result = global + local;
    exit result;
}
```

### Complex Expressions

```
const result = (10 + 5) * 2 - 3;
exit result == 27;  # Exits with code 1 (true)
```

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature-name`
3. Make your changes and add tests
4. Commit your changes: `git commit -am 'Add new feature'`
5. Push to the branch: `git push origin feature-name`
6. Submit a pull request

## Future Enhancements

- [ ] Implement loops (`while`, `for`)
- [ ] Add function definitions and calls
- [ ] Support for more data types (strings, booleans)
- [ ] Enhanced error reporting with line numbers
- [ ] Optimization passes for generated assembly
- [ ] Support for additional target architectures

## License

This project is open source. See the repository for license details.

## Author

Created by [02YashRajput](https://github.com/02YashRajput)

# CoolCompiler

A compiler for the COOL language (Stanford spec) generating LLVM IR. Built with C++17, LLVM, and CMake.

Compilation Pipeline
COOL Source → Lexer → Parser → AST → CodeGen → LLVM IR → Executable

## Build

```bash
git clone https://github.com/jithangowda/CoolCompiler.git
cd CoolCompiler
cmake -B build
cmake --build build
```

### Compile a COOL Program

```bash
./build/coolc examples/maths.cl      # Generates IR_maths.ll
clang IR_maths.ll -o program         # Creates executable
./program                            # Runs the program

```
### Example COOL Code

```coolc
class Main {
  a : Int <- 12;
  b : Int <- 4;

  main() : Int {
    {
      -- Hello message
      (new IO).out_string("=== Cool Compiler Demo ===\n");
      
      -- Arithmetic operations
      (new IO).out_string(a);
      (new IO).out_string(" + ");
      (new IO).out_string(b);
      (new IO).out_string(" = ");
      (new IO).out_int(a + b);
      (new IO).out_string("\n");
      
      (new IO).out_string(a);
      (new IO).out_string(" - ");
      (new IO).out_string(b);
      (new IO).out_string(" = ");
      (new IO).out_int(a - b);
      (new IO).out_string("\n");
      
      0;
    }
  };
};
```

### Usage

```bash
./build/coolc <input.cl> [output_dir]

Examples:

./build/coolc program.cl             # Creates IR_program.ll
./build/coolc program.cl ./output    # Creates output/IR_program.ll
```

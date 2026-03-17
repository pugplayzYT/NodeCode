<div align="center">
  <h1>NodeCode Studio</h1>
  <p>A powerful node-based development environment with support for code generation and multi-language execution (C++, JavaScript, Python).</p>
  
  [![CI Build & Test](https://github.com/pugplayzYT/NodeCode/actions/workflows/ci.yml/badge.svg)](https://github.com/pugplayzYT/NodeCode/actions/workflows/ci.yml)
  [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

</div>

## Features ✨
- **Multi-language Support:** Visual logic compiles natively into C++, JavaScript, and Python.
- **Real-time Code Generation:** See the translated script instantly as you connect nodes.
- **Robust Typing System:** Variables are strictly typed with support for various native data structures (Strings, Ints, Vectors, Arrays).
- **Control Flow Logic:** Built-in conditional (If/Else) and looping (While) logic nodes.
- **Node Development:** Extensible `.node` definition schema to integrate your logic block libraries swiftly.

## Getting Started 🚀

### Prerequisites
- C++17 Compatible Compiler (GCC/Clang/MSVC)
- CMake 3.16+
- Qt 6 (Core, Gui, Widgets)

### Build Instructions

```bash
git clone https://github.com/your-username/NodeCode.git
cd NodeCode

# Configure and generate build files
cmake -B build -DBUILD_TESTING=ON

# Build the project
cmake --build build -j$(nproc)

# Run tests
cd build && ctest --output-on-failure
```

### Running 
Launch the Studio after build:
```bash
./build/NodeCode
```

## Creating new Nodes 🧩

You can add new modules to `nodes/` by providing a JSON definition. Example C++ Node:
```json
{
  "name": "CustomMath",
  "category": "Math",
  "language": "cpp",
  "inputs": [{"name": "A", "type": "number"}],
  "outputs": [{"name": "Result", "type": "number"}],
  "code": "%Result% = %A% * 10;"
}
```

## Contributing 🤝

Contributions are welcome! Please ensure any new features are accompanied by tests.
Our CI/CD pipeline enforces passing tests before any branch can be merged into the `main` stream.

## License 📄
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

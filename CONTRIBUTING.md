# Contributing to Stinger C++ Utils

Thank you for your interest in contributing! Here are some guidelines to help maintain code quality.

## Code Style

This project uses `clang-format` to maintain consistent code style. Before submitting a pull request:

1. **Install clang-format**:
   ```bash
   sudo apt-get install clang-format
   ```

2. **Format your code**:
   ```bash
   find include src tests examples -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i
   ```

3. **Commit the formatted code**:
   ```bash
   git add -u
   git commit
   ```

The CI pipeline will automatically check code formatting on pull requests. If the check fails, simply run the formatting command above and push the changes.

## Building and Testing

Before submitting your changes:

1. **Build the project**:
   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

2. **Run tests**:
   ```bash
   ctest --output-on-failure
   ```

3. **Ensure all tests pass** before creating a pull request.

## Pre-commit Hook (Optional)

You can automatically format code before each commit by setting up a pre-commit hook:

```bash
cat > .git/hooks/pre-commit << 'EOF'
#!/bin/bash
# Format staged C++ files
FILES=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|hpp)$')
if [ -n "$FILES" ]; then
    clang-format -i $FILES
    git add $FILES
fi
EOF

chmod +x .git/hooks/pre-commit
```

## Pull Request Process

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/my-feature`)
3. Make your changes
4. Format your code with clang-format
5. Ensure tests pass
6. Commit your changes (`git commit -m 'Add some feature'`)
7. Push to the branch (`git push origin feature/my-feature`)
8. Open a Pull Request

Thank you for contributing!

# Contributing Guide

Thank you for your interest in contributing to HTS! We welcome contributions of all kinds: code, documentation, examples, and bug reports.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [How to Contribute](#how-to-contribute)
  - [Reporting Bugs](#reporting-bugs)
  - [Suggesting Features](#suggesting-features)
  - [Code Contributions](#code-contributions)
  - [Documentation](#documentation)
  - [Examples](#examples)
- [Development Workflow](#development-workflow)
  - [Setup Development Environment](#setup-development-environment)
  - [Branching Strategy](#branching-strategy)
  - [Commit Messages](#commit-messages)
  - [Pull Requests](#pull-requests)
- [Coding Standards](#coding-standards)
- [Testing](#testing)
- [Code Review](#code-review)
- [Community](#community)

---

## Code of Conduct

This project follows the [Contributor Covenant Code of Conduct](https://www.contributor-covenant.org/). Please read it before participating.

**Our pledge:** Create an open, welcoming, diverse, inclusive, and healthy community.

---

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** to your local machine
   ```bash
   git clone https://github.com/YOUR_USERNAME/heterogeneous-task-scheduler.git
   cd heterogeneous-task-scheduler
   ```
3. **Set up upstream remote**
   ```bash
   git remote add upstream https://github.com/LessUp/heterogeneous-task-scheduler.git
   ```
4. **Build the project**
   ```bash
   mkdir build && cd build
   cmake .. -DHTS_BUILD_TESTS=ON -DHTS_BUILD_EXAMPLES=ON
   make -j$(nproc)
   ```
5. **Run tests** to verify your setup
   ```bash
   ctest --output-on-failure
   ```

---

## How to Contribute

### Reporting Bugs

Before reporting a bug:

1. **Search existing issues** to avoid duplicates
2. **Reproduce with the latest version**
3. **Gather information**:
   - HTS version
   - OS and compiler version
   - CUDA version (if applicable)
   - Minimal reproducible example

**Create an issue** with:
- Clear, descriptive title
- Steps to reproduce
- Expected vs. actual behavior
- System information
- Error logs or stack traces

**Example:**
```markdown
**Title**: Deadlock when using RetryPolicy with GPU tasks

**Description**:
When a GPU task with RetryPolicy fails, the scheduler sometimes deadlocks...

**Steps to Reproduce**:
1. Create GPU task with RetryPolicy
2. Make task fail intentionally
3. Retry triggers deadlock

**Expected**: Task retries 3 times
**Actual**: Scheduler hangs after first retry

**Environment**:
- HTS v1.2.0
- Ubuntu 22.04, GCC 11.3
- CUDA 12.1
```

### Suggesting Features

We love feature suggestions! Please:

1. **Check if it's already planned** in our roadmap
2. **Describe the use case** clearly
3. **Explain the benefit** to users
4. **Provide examples** if possible

**Good feature request:**
```markdown
**Title**: Add support for task priorities with ranges

**Use Case**: 
I'm building a pipeline with 100+ tasks and need fine-grained priority control...

**Benefit**:
Would allow better scheduling decisions for complex DAGs...

**Example**:
```cpp
task->set_priority_range(1, 100);  // Dynamic priority
```
```

### Code Contributions

**Before coding:**

1. **Find or create an issue** describing the change
2. **Discuss the approach** with maintainers if needed
3. **Check for related work** in progress

**What we accept:**

- ✅ Bug fixes
- ✅ Performance improvements
- ✅ New features (discussed first)
- ✅ Documentation improvements
- ✅ Examples and tutorials
- ✅ Test coverage improvements

**What we don't accept:**

- ❌ Unrelated refactoring without discussion
- ❌ Code without tests
- ❌ Breaking changes without migration path
- ❌ Features that complicate the API without clear benefit

### Documentation

Documentation is just as important as code! You can contribute by:

- **Fixing typos** and grammar errors
- **Improving explanations** in guides
- **Adding examples** to API docs
- **Creating tutorials**
- **Translating documentation**

**Documentation standards:**
- Use Markdown format
- Follow existing structure
- Include code examples
- Keep it concise but complete

### Examples

We especially welcome example contributions!

**Guidelines:**
- Place in `examples/` directory
- Include a `README.md` with explanation
- Add to CMakeLists.txt
- Follow coding standards
- Test thoroughly

**Example template:**
```
examples/
└── your_example/
    ├── README.md       # What it does and how to run
    ├── CMakeLists.txt  # Build configuration
    └── main.cpp        # Example code
```

---

## Development Workflow

### Setup Development Environment

**Required tools:**
- CMake 3.18+
- GCC 7+ or Clang 5+ or MSVC 2017+
- CUDA Toolkit 11.0+ (optional, for GPU support)
- Git
- clang-format-17 (for code formatting)
- clang-tidy-17 (for static analysis)

**Install dependencies (Ubuntu):**
```bash
sudo apt install build-essential cmake git clang-format-17 clang-tidy-17
```

**Install dependencies (macOS):**
```bash
brew install cmake git llvm
```

### Branching Strategy

We use a simple branching model:

- **`main`**: Stable release branch
- **Feature branches**: `feature/your-feature`
- **Bug fix branches**: `fix/issue-description`
- **Release branches**: `release/v1.x.x`

**Workflow:**
```bash
# Create feature branch
git checkout -b feature/my-feature main

# Make changes and commit
git add .
git commit -m "feat: add my awesome feature"

# Push to your fork
git push origin feature/my-feature

# Create Pull Request
```

### Commit Messages

We follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <description>

[optional body]

[optional footer(s)]
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting)
- `refactor`: Code refactoring
- `perf`: Performance improvements
- `test`: Test additions or changes
- `chore`: Build system or tooling changes

**Examples:**
```
feat(scheduler): add load-based scheduling policy

fix(memory-pool): fix fragmentation calculation bug

docs(guide): update installation instructions

perf(execution): reduce scheduling overhead by 30%

test(task): add unit tests for task builder
```

**Best practices:**
- Use imperative mood ("add" not "added")
- Keep first line under 72 characters
- Add body for complex changes explaining WHY
- Reference issues: `Closes #123`

### Pull Requests

**Before submitting:**

1. **Rebase on latest main**
   ```bash
   git fetch upstream
   git rebase upstream/main
   ```

2. **Format your code**
   ```bash
   scripts/format.sh
   ```

3. **Run tests**
   ```bash
   scripts/test.sh
   ```

4. **Run static analysis**
   ```bash
   scripts/analyze.sh
   ```

**PR checklist:**
- [ ] Code follows style guidelines
- [ ] Tests added/updated
- [ ] Documentation updated
- [ ] Commit messages follow convention
- [ ] No breaking changes (or migration documented)

**PR template:**
```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
- [ ] Tests pass locally
- [ ] Added new tests
- [ ] Manual testing performed

## Checklist
- [ ] Code formatted (scripts/format.sh)
- [ ] Tests pass (scripts/test.sh)
- [ ] Documentation updated
```

---

## Coding Standards

### C++ Style

**Naming:**
- Classes: `PascalCase`
- Functions: `snake_case`
- Variables: `snake_case`
- Constants: `UPPER_CASE`
- Private members: `snake_case_` (trailing underscore)

**Formatting:**
- 4 spaces indentation
- 100 character line limit
- Use clang-format (configuration in `.clang-format`)
- Braces on same line (Attach style)

**Example:**
```cpp
class TaskGraph {
public:
    std::shared_ptr<Task> add_task(DeviceType device, const std::string& name);
    size_t num_tasks() const;
    
private:
    void validate_graph_();
    std::vector<std::shared_ptr<Task>> tasks_;
};
```

### Documentation

**Doxygen style for public API:**
```cpp
/// @brief Creates a new task in the graph
/// @param device The device type (CPU or GPU)
/// @param name Human-readable task name
/// @return Shared pointer to the created task
/// @throws std::invalid_argument if name is empty
std::shared_ptr<Task> add_task(DeviceType device, const std::string& name);
```

### Headers

**Include order:**
1. Related header (for .cpp files)
2. C++ standard library
3. Third-party libraries
4. Project headers

**Use forward declarations** when possible to reduce compile times.

---

## Testing

### Running Tests

```bash
# Run all tests
scripts/test.sh

# Run specific test
scripts/test.sh --pattern "test_scheduler"

# Run with verbose output
scripts/test.sh --verbose

# Generate coverage report
scripts/test.sh --coverage
```

### Writing Tests

**Test organization:**
- `tests/unit/` - Unit tests for individual components
- `tests/integration/` - Integration tests

**Test naming:**
```cpp
TEST(TaskTest, CreateTaskWithValidName) {
    // Test code
}

TEST(TaskGraphTest, AddDependency_WhenCycleCreated_ThrowsError) {
    // Test code
}
```

**Test structure (Arrange-Act-Assert):**
```cpp
TEST(SchedulerTest, Execute_CompletesAllTasks_ReturnsSuccess) {
    // Arrange
    TaskGraph graph;
    auto task = graph.add_task(DeviceType::CPU);
    
    // Act
    Scheduler scheduler;
    scheduler.init(&graph);
    scheduler.execute();
    scheduler.wait_for_completion();
    
    // Assert
    EXPECT_TRUE(scheduler.is_completed());
    EXPECT_EQ(scheduler.get_stats().tasks_scheduled, 1);
}
```

**Coverage requirements:**
- New features: > 80% coverage
- Bug fixes: Include regression test
- Critical paths: 100% coverage

---

## Code Review

All PRs require review before merging.

**Review checklist:**
- [ ] Code is correct and complete
- [ ] Tests are adequate
- [ ] Performance impact assessed
- [ ] Security implications considered
- [ ] Documentation updated

**Review process:**
1. Maintainer reviews PR
2. Requests changes if needed
3. Author addresses feedback
4. Maintainer approves
5. Maintainer merges

**Typical review time:** 1-3 business days

---

## Community

**Get involved:**
- 💬 [GitHub Discussions](https://github.com/LessUp/heterogeneous-task-scheduler/discussions)
- 🐛 [Issue Tracker](https://github.com/LessUp/heterogeneous-task-scheduler/issues)
- 📖 [Documentation](https://lessup.github.io/heterogeneous-task-scheduler/)

**Contact maintainers:**
- For questions: Use Discussions
- For bugs: Create an issue
- For security: Email security@ (see SECURITY.md)

---

## Recognition

All contributors are recognized in:
- [CONTRIBUTORS.md](CONTRIBUTORS.md)
- Release notes
- Project website

**Thank you for contributing to HTS!** 🎉

---

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

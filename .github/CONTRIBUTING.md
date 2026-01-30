# Contributing to Yes UE FSD

Thank you for your interest in contributing to Yes UE FSD! This document provides guidelines for contributing to the project.

## Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/YOUR_USERNAME/yes-ue-fsd.git`
3. Create a feature branch: `git checkout -b feature/your-feature-name`
4. Make your changes
5. Test thoroughly
6. Commit with a descriptive message
7. Push to your fork
8. Create a Pull Request

## Development Workflow

### 1. Create an Issue First

Before starting work on a new feature or bug fix:

1. **Check existing issues** to avoid duplicates
2. **Create a new issue** describing:
   - What you want to implement/fix
   - Why it's needed
   - Your proposed approach
3. **Wait for discussion** - maintainers may have suggestions
4. **Get assigned** to the issue before starting work

### 2. Branch Naming

Use descriptive branch names following this pattern:

- `feature/description` - New features
- `fix/description` - Bug fixes
- `docs/description` - Documentation updates
- `refactor/description` - Code refactoring
- `perf/description` - Performance improvements

Examples:
- `feature/action-recording`
- `fix/movement-timeout`
- `docs/api-reference`

### 3. Commit Messages

Follow the [Conventional Commits](https://www.conventionalcommits.org/) format:

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

**Types:**
- `feat` - New feature
- `fix` - Bug fix
- `docs` - Documentation
- `refactor` - Code refactoring
- `perf` - Performance improvement
- `test` - Adding tests
- `chore` - Maintenance tasks

**Examples:**
```
feat(command): Add WaitForDurationCommand

Implements a new command that pauses automation for a specified duration.
This is useful for creating timed action sequences.

Closes #42
```

```
fix(movement): Fix infinite loop in MoveToLocationCommand

The command was not properly checking timeout when using direct movement mode.
Added timeout check in tick function.

Fixes #38
```

### 4. Code Style

**C++ Guidelines:**
- Follow [Unreal Engine C++ Coding Standard](https://docs.unrealengine.com/5.0/en-US/epic-cplusplus-coding-standard-for-unreal-engine/)
- Use clear, descriptive names
- Add comments for complex logic
- Document public APIs with Doxygen-style comments
- Prefer `const` where possible
- Use `nullptr` instead of `NULL`

**Header Organization:**
```cpp
// 1. Copyright notice
// 2. #pragma once
// 3. System includes
// 4. UE includes
// 5. Project includes
// 6. Forward declarations
// 7. Class definition
```

**Naming Conventions:**
- Classes: `UAutoDriverComponent`, `FAutoDriverCommandBase`
- Functions: `MoveToLocation()`, `GetTargetActor()`
- Variables: `bIsRunning`, `TargetLocation`, `PlayerController`
- Constants: `DEFAULT_TIMEOUT`, `MAX_SPEED`

### 5. Testing

Before submitting a PR:

- [ ] Build succeeds in Development Editor configuration
- [ ] No new compiler warnings
- [ ] Test in PIE (Play-In-Editor)
- [ ] Test with Blueprint if API is Blueprint-exposed
- [ ] Check for memory leaks with Unreal's memory tools
- [ ] Document any new dependencies

### 6. Documentation

When adding new features:

- [ ] Update `README.md` if needed
- [ ] Add usage examples to `USAGE.md`
- [ ] Add code comments for public APIs
- [ ] Update inline documentation
- [ ] Add TODO comments for incomplete features

### 7. Pull Request Process

1. **Fill out the PR template** completely
2. **Link related issues** using "Closes #123" or "Fixes #456"
3. **Add screenshots/videos** if the change is visual
4. **Wait for review** - be patient and responsive to feedback
5. **Make requested changes** in new commits
6. **Squash commits** if requested before merge

## Code Review Guidelines

When reviewing PRs:

- Be constructive and respectful
- Focus on code quality, not personal preferences
- Suggest improvements, don't demand them
- Approve if the code is good enough (perfect is the enemy of done)

## Issue Management

### Creating Issues

**For Bugs:**
- Use the bug report template
- Include reproduction steps
- Provide logs and error messages
- Specify UE version and platform

**For Features:**
- Use the feature request template
- Explain the use case
- Describe the desired behavior
- Consider implementation complexity

### Working on Issues

1. Comment on the issue to claim it
2. Wait for assignment from maintainers
3. Keep the issue updated with progress
4. Close the issue with your PR using "Closes #123"

## Project Structure

```
yes-ue-fsd/
├── .github/              # GitHub templates and workflows
├── Config/               # Plugin configuration
├── Source/
│   ├── YesUeFsd/        # Runtime module
│   │   ├── Public/      # Public headers
│   │   └── Private/     # Implementation files
│   └── YesUeFsdEditor/  # Editor module
├── Resources/           # Icons and assets
├── README.md            # Main documentation
├── USAGE.md            # Usage guide
└── YesUeFsd.uplugin    # Plugin manifest
```

## Need Help?

- Check existing issues and documentation
- Ask questions in issue comments
- Join discussions in the repository

## License

By contributing, you agree that your contributions will be licensed under the same license as the project.

---

Thank you for contributing to Yes UE FSD! 🚀

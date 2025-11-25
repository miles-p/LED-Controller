# LED Controller - Build Testing

This repository includes automated build testing on every commit.

## Automated Build Testing

### GitHub Actions (CI/CD)

- **Trigger**: Runs on every push and pull request to any branch
- **Location**: `.github/workflows/build.yml`
- **What it does**:
  - Checks out code
  - Sets up PlatformIO environment
  - Builds firmware for `megaatmega2560` target
  - Uploads firmware artifact (`.hex` file)
  - Caches dependencies for faster builds

### Local Pre-commit Hook

- **Trigger**: Runs automatically before each git commit
- **Location**: `.git/hooks/pre-commit`
- **What it does**:
  - Runs `pio run` to build the project
  - Blocks commit if build fails
  - Allows commit to proceed if build succeeds

## Build Status

Check the **Actions** tab on GitHub to see build status for all commits.

## Manual Testing

To manually test the build:

```bash
pio run
```

To skip the pre-commit hook (not recommended):

```bash
git commit --no-verify
```

## Requirements

- PlatformIO Core installed locally for pre-commit hooks
- GitHub Actions handles CI builds automatically (no local setup needed)

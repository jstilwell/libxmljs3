#!/usr/bin/env bash
set -euo pipefail

# Deploy script: build, test, tag, push, and publish to npm
# Usage: ./bin/deploy.sh <patch|minor|major>

BUMP="${1:-}"

if [[ -z "$BUMP" ]]; then
  echo "Usage: ./bin/deploy.sh <patch|minor|major>"
  exit 1
fi

if [[ "$BUMP" != "patch" && "$BUMP" != "minor" && "$BUMP" != "major" ]]; then
  echo "Error: version bump must be 'patch', 'minor', or 'major'"
  exit 1
fi

# Ensure clean working tree
if [[ -n "$(git status --porcelain)" ]]; then
  echo "Error: working tree is not clean. Commit or stash changes first."
  exit 1
fi

BRANCH="$(git branch --show-current)"

# Build and test on current branch before merging
echo "Building native addon..."
pnpm run build

echo "Building TypeScript..."
pnpm run build:ts

echo "Running tests..."
pnpm test

# Merge into main if on a feature branch
if [[ "$BRANCH" != "main" ]]; then
  echo "Switching to main and merging '$BRANCH'..."
  git checkout main
  git pull --ff-only origin main
  git merge --no-ff "$BRANCH" -m "Merge branch '$BRANCH'"
  git branch -d "$BRANCH"
else
  echo "Pulling latest from origin..."
  git pull --ff-only origin main
fi

# Bump version (updates package.json and creates git tag)
echo "Bumping $BUMP version..."
NEW_VERSION="$(npm version "$BUMP" -m "v%s")"
echo "New version: $NEW_VERSION"

# Push commit and tag
echo "Pushing to origin..."
git push origin main
git push origin "$NEW_VERSION"

# Publish to npm
echo "Publishing to npm..."
pnpm publish --access public --no-git-checks

echo ""
echo "Deployed $NEW_VERSION"
echo "  - Git tag pushed (CI will create GitHub Release with prebuilds)"
echo "  - Published to npm: https://www.npmjs.com/package/libxmljs4"

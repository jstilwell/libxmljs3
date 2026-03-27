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

# Ensure we're on main
BRANCH="$(git branch --show-current)"
if [[ "$BRANCH" != "main" ]]; then
  echo "Error: must be on main branch (currently on '$BRANCH')"
  exit 1
fi

# Pull latest
echo "Pulling latest from origin..."
git pull --ff-only origin main

# Build and test
echo "Building native addon..."
pnpm run build

echo "Building TypeScript..."
pnpm run build:ts

echo "Running tests..."
pnpm test

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

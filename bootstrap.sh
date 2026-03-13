#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(git -C "$SCRIPT_DIR" rev-parse --show-toplevel)"

UPSTREAM_URL="https://gitlab.kitware.com/cmake/cmake.git"

# Add upstream remote
if git -C "$REPO_ROOT" remote get-url upstream >/dev/null 2>&1; then
    echo "upstream remote already configured: $(git -C "$REPO_ROOT" remote get-url upstream)"
else
    git -C "$REPO_ROOT" remote add upstream "$UPSTREAM_URL"
    echo "Added upstream remote: $UPSTREAM_URL"
fi

# Fetch upstream tags
echo "Fetching upstream tags..."
git -C "$REPO_ROOT" fetch upstream --tags --no-recurse-submodules
echo "Done."

echo
echo "Recent CMake release tags:"
git -C "$REPO_ROOT" tag -l 'v[0-9]*' --sort=-v:refname \
    | grep -v -- '-rc' \
    | head -10

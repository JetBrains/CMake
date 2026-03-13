# JetBrains CMake Patches

This branch contains patches applied to CMake for JetBrains IDE builds (CLion).

## Structure

```
(utils/patches branch root)
├── bootstrap.sh       # Set up upstream remote and fetch release tags
├── apply.sh           # Apply patches to a release tag
├── manifests/         # Versioned manifests (flat patch lists)
│   ├── 4.manifest     # Patch list for CMake 4.x (2 patches)
│   └── 4.1.manifest   # Patch list for CMake 4.1.x (24 patches, includes debugger)
└── patches/           # Patch files
    ├── dbg-*.patch    # CMake debugger patches (20 files)
    ├── macro-args.patch
    ├── env-vars.patch
    ├── cygport-move.patch
    └── cygwin-parallel-bootstrap.patch
```

## Versioned manifests

Each CMake version range has a manifest file listing patches in application order.

Resolution order for CMake 4.3.0:
1. `manifests/4.3.0.manifest` — exact version
2. `manifests/4.3.manifest` — minor version
3. `manifests/4.manifest` — major version fallback

The CMake debugger was upstreamed in 4.2, so `4.manifest` only has 2 non-debugger patches while `4.1.manifest` has all 24 (including 20 debugger patches).

## Usage

### First-time setup

```bash
git checkout utils/patches
./bootstrap.sh
```

### Apply patches

```bash
./apply.sh v4.3.0                  # → v4.3.0-clion branch
./apply.sh v4.3.0 v4.3.0-clion    # explicit branch name
./apply.sh -f v4.3.0 --push       # force recreate + push
./apply.sh                         # interactive mode
```

### When patches fail

If a patch doesn't apply cleanly, `apply.sh` leaves the worktree with `.rej` files and generates:

- **`resume.sh`** (in worktree) — run after fixing rejects to commit and continue
- **`claudefix.sh`** (in script dir) — launches Claude Code to fix rejects automatically

```bash
# Option A: fix manually
cd /tmp/cmake-patches-XXXXXX
# fix rejects, delete .rej files
./resume.sh

# Option B: let Claude fix it
./claudefix.sh
```

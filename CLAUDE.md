# Patches Branch — AI Assistant Instructions

This is the `utils/patches` branch of `jetbrains-cmake`. It contains JetBrains-specific patches for CMake, applied on top of upstream release tags.

## Applying patches

```bash
# Apply patches for a CMake release (resolves manifest automatically)
./apply.sh v4.3.0

# Explicit branch name
./apply.sh v4.3.0 v4.3.0-clion

# Recreate existing branch
./apply.sh -f v4.3.0

# Apply and push
./apply.sh -f v4.3.0 --push
```

`apply.sh` extracts the CMake version from the tag, resolves the manifest from `manifests/`, applies patches in order, then copies runtime params (if any). One output branch: `v{ver}-clion`.

## Versioned manifests

Manifests live in `manifests/` as flat patch lists (no platform sections).

Resolution for CMake 4.3.0: `manifests/4.3.0.manifest` → `manifests/4.3.manifest` → `manifests/4.manifest`.

## Runtime params

`runtime-params/` contains TeamCity build configuration (e.g., OpenSSL version). Copied to `.teamcity-runtime-params/` on the output branch. Same version fallback as manifests: `runtime-params/4.3.0/` → `runtime-params/4.3/` → `runtime-params/4/`. Skipped if no matching directory exists.

## Adding a new patch

1. Create the `.patch` file (`git format-patch` from the patched branch)
2. Place it in `patches/`
3. Add it to the relevant manifest(s)
4. Commit to this branch

## Adding a new CMake version

1. Copy the latest manifest → `manifests/<new-version>.manifest`
2. Test each patch against the new CMake source
3. Create version-specific patch variants where needed
4. If a minor version diverges, add `manifests/<major>.<minor>.manifest`

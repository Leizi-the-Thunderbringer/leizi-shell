# AUR Submission Guide for leizi-shell

## Prerequisites

1. **AUR Account**: Register at https://aur.archlinux.org/register
2. **SSH Key**: Configure SSH key in your AUR account settings

## Step-by-Step Submission Process

### 1. Generate SSH Key (if needed)

```bash
ssh-keygen -t ed25519 -C "your_email@example.com"
cat ~/.ssh/id_ed25519.pub  # Copy this and add to AUR account
```

### 2. Clone AUR Repository

```bash
# First time submission (package doesn't exist yet)
git clone ssh://aur@aur.archlinux.org/leizi-shell.git aur-leizi-shell
cd aur-leizi-shell

# If package already exists
git clone ssh://aur@aur.archlinux.org/leizi-shell.git
cd leizi-shell
```

### 3. Copy Files

```bash
# Copy PKGBUILD and .SRCINFO
cp /path/to/leizi-shell/packaging/arch/PKGBUILD .
cp /path/to/leizi-shell/packaging/arch/.SRCINFO .
```

### 4. Verify PKGBUILD

```bash
# Test build locally
makepkg -si

# Check with namcap (optional)
namcap PKGBUILD
namcap leizi-shell-*.pkg.tar.zst
```

### 5. Commit and Push

```bash
git add PKGBUILD .SRCINFO
git commit -m "Initial commit: leizi-shell 1.4.0"
git push origin master
```

### 6. Verify Submission

Visit: https://aur.archlinux.org/packages/leizi-shell

## Updating the Package

When releasing a new version:

```bash
cd aur-leizi-shell

# Update PKGBUILD version and checksums
sed -i 's/pkgver=.*/pkgver=NEW_VERSION/' PKGBUILD
sed -i 's/sha256sums=.*/sha256sums=('NEW_SHA256')/' PKGBUILD

# Regenerate .SRCINFO
makepkg --printsrcinfo > .SRCINFO

# Commit and push
git add PKGBUILD .SRCINFO
git commit -m "Update to NEW_VERSION"
git push
```

## Current Version Info

- **Package Name**: leizi-shell
- **Version**: 1.4.0
- **SHA256**: `8241ed63466189da7a1f6269500bf88b67c48661ed04236cb7d4646188a3e314`
- **Source URL**: https://github.com/Zixiao-System/leizi-shell/archive/v1.4.0.tar.gz

## Notes

- AUR submission requires an active AUR account
- SSH access is mandatory (no HTTPS)
- `.SRCINFO` must be kept in sync with PKGBUILD
- Test build locally before pushing
- Follow AUR submission guidelines: https://wiki.archlinux.org/title/AUR_submission_guidelines
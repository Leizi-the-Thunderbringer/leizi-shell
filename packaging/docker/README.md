# Leizi Shell Docker Image

This directory contains the Dockerfile for building Leizi Shell Docker images.

## Quick Start

### Pull from Docker Hub (when available)
```bash
docker pull leizi/leizi-shell:latest
docker run -it leizi/leizi-shell:latest
```

### Build Locally
```bash
# From the project root directory
docker build -t leizi-shell -f packaging/docker/Dockerfile .
```

### Run
```bash
# Interactive shell
docker run -it leizi-shell

# Run a command
docker run --rm leizi-shell -c "echo hello world"

# Mount a volume for persistent config
docker run -it -v ~/.config/leizi:/home/leizi/.config/leizi leizi-shell
```

## Multi-Architecture Support

The official images support both `amd64` and `arm64` architectures.

### Build for Multiple Architectures
```bash
# Setup buildx
docker buildx create --name multiarch --use

# Build and push
docker buildx build \
  --platform linux/amd64,linux/arm64 \
  -t leizi/leizi-shell:1.4.0 \
  -t leizi/leizi-shell:latest \
  --push \
  -f packaging/docker/Dockerfile .
```

## Image Details

- **Base Image**: Ubuntu 22.04
- **Shell**: Leizi Shell v1.4.0
- **User**: leizi (non-root)
- **Config Directory**: `/home/leizi/.config/leizi`
- **Size**: ~150MB (compressed)

## Environment Variables

- `TERM`: Set to `xterm-256color` for color support
- `SHELL`: Set to `/usr/local/bin/leizi`

## Use Cases

### Development Environment
```bash
docker run -it -v $(pwd):/workspace -w /workspace leizi-shell
```

### CI/CD Testing
```bash
docker run --rm leizi-shell -c "leizi --version"
```

### Try Before Install
```bash
docker run -it leizi-shell
```

## Customization

You can create a custom Dockerfile based on this image:

```dockerfile
FROM leizi/leizi-shell:latest

# Copy custom config
COPY my-config /home/leizi/.config/leizi/config

# Install additional tools
USER root
RUN apt-get update && apt-get install -y vim tmux
USER leizi
```

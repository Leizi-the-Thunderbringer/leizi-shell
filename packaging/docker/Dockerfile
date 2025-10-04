# Multi-stage build for Leizi Shell
FROM ubuntu:22.04 as builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libreadline-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

# Copy source code
WORKDIR /src
COPY . .

# Build Leizi Shell
RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc)

# Runtime stage
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libreadline8 \
    git \
    && rm -rf /var/lib/apt/lists/* \
    && useradd -m -s /bin/bash leizi

# Copy binary from builder
COPY --from=builder /src/build/leizi /usr/local/bin/

# Add shell to /etc/shells
RUN echo "/usr/local/bin/leizi" >> /etc/shells

# Create config directory
RUN mkdir -p /home/leizi/.config/leizi && \
    chown -R leizi:leizi /home/leizi/.config

# Switch to leizi user
USER leizi
WORKDIR /home/leizi

# Set environment
ENV TERM=xterm-256color
ENV SHELL=/usr/local/bin/leizi

# Entry point
ENTRYPOINT ["/usr/local/bin/leizi"]

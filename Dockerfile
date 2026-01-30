# Build stage
FROM rust:1.75-slim-bookworm as builder

# Install system dependencies for Tauri build
RUN apt-get update && apt-get install -y \
    libgtk-3-dev \
    libwebkit2gtk-4.1-dev \
    libappindicator3-dev \
    librsvg2-dev \
    patchelf \
    libsoup-3.0-dev \
    curl \
    build-essential \
    pkg-config \
    libssl-dev \
    javascriptcore-rs-sys \
    git \
    && rm -rf /var/lib/apt/lists/*

# Install Node.js
RUN curl -fsSL https://deb.nodesource.com/setup_20.x | bash - && \
    apt-get install -y nodejs

WORKDIR /app

# Copy files
COPY . .

# Install frontend dependencies
RUN npm install

# Build the application
RUN npm run tauri build

# Final stage - for a GUI app in Docker, we typically use a VNC-based setup 
# if we want to run it, but here we'll just provide a base to hold the artifact 
# or run in a minimal X11 environment.
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y \
    libgtk-3-0 \
    libwebkit2gtk-4.1-0 \
    libsoup-3.0-0 \
    libjavascriptcoregtk-4.1-0 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/src-tauri/target/release/modufile /usr/local/bin/modufile

# Set the entrypoint
ENTRYPOINT ["/usr/local/bin/modufile"]

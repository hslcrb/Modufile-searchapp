# Build stage
FROM ubuntu:22.04 as builder

# Avoid prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies and Qt 6
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    qt6-base-dev \
    qt6-base-private-dev \
    libgl1-mesa-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# Build the application
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build --config Release

# Final stage - Minimal runtime
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libqt6widgets6 \
    libqt6gui6 \
    libqt6core6 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/modufile /usr/local/bin/modufile

# Entrypoint
ENTRYPOINT ["/usr/local/bin/modufile"]

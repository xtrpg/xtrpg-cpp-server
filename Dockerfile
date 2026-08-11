# ==========================================
# STAGE 1: Builder (Compilers + Dependencies)
# ==========================================
FROM ubuntu:24.04 AS builder

# Prevent interactive prompts during apt installs
ENV DEBIAN_FRONTEND=noninteractive
ENV VCPKG_ROOT=/opt/vcpkg

# Install native build tools
RUN apt-get update && apt-get install -y --no-install-recommends \
	build-essential \
	g++ \
	cmake \
	ninja-build \
	git \
	curl \
	zip \
	unzip \
	tar \
	pkg-config \
	ca-certificates \
	&& rm -rf /var/lib/apt/lists/*

# Clone vcpkg executable
RUN git clone https://github.com/microsoft/vcpkg.git $VCPKG_ROOT && \
	$VCPKG_ROOT/bootstrap-vcpkg.sh -disableMetrics

WORKDIR /workspace

# Copy manifest file first to leverage Docker layer caching for dependencies
COPY vcpkg.json .

# Pre-fetch and compile dependencies (OpenSSL, Asio, PugiXML)
# This layer stays cached unless vcpkg.json changes!
RUN $VCPKG_ROOT/vcpkg install --triplet x64-linux

# Copy complete project source code
COPY . .

# Build project with CMake and Ninja
ARG ENABLE_C2S=ON
ARG ENABLE_S2S=ON
ARG ENABLE_STORAGE_SQLITE=OFF

RUN cmake -B build -S . -G Ninja \
	-DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
	-DCMAKE_BUILD_TYPE=Release \
	-DENABLE_C2S=${ENABLE_C2S} \
	-DENABLE_S2S=${ENABLE_S2S} \
	-DENABLE_STORAGE_SQLITE=${ENABLE_STORAGE_SQLITE} \
	&& cmake --build build --config Release

# ==========================================
# STAGE 2: Minimal Runtime Container
# ==========================================
FROM ubuntu:24.04 AS runner

# Install only runtime SSL certificates / basic libraries
RUN apt-get update && apt-get install -y --no-install-recommends \
	ca-certificates \
	libstdc++6 \
	&& rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy built binary from builder stage
COPY --from=builder /workspace/build/xtrpg-cpp-server /app/xtrpg-cpp-server

# Expose C2S (5222) and S2S (5269) ports
EXPOSE 5222 5269

ENTRYPOINT ["/app/xtrpg-cpp-server"]
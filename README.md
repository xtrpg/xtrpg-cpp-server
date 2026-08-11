# xtrpg-cpp-server
XMPP Server written in C++ built specifically around the XTRPG specifications.

## Build Command across Operating Systems

```bash
# Set VCPKG_ROOT environment variable pointing to your vcpkg installation
# Linux / macOS / Windows
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```



## Dockerfile

```shell
# Build the Docker image locally
docker build -t xmpp-builder .

# Extract the compiled binary out of Docker onto the host OS
docker run --rm -v $(pwd)/dist:/output xmpp-builder cp /app/xmpp_server /output/
```

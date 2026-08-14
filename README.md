# XTRPG C++ Server

XMPP Server written in C++ built specifically around the XTRPG specifications.

## 🛠️ Project Setup & Installation

This project uses **CMake** (v3.20+) and **vcpkg** in **Manifest Mode** (`vcpkg.json`) to manage dependencies (such as Asio, OpenSSL, and pugixml) across Windows, Linux, and macOS.

---

### 1. Prerequisites

Before building the project, ensure you have the following installed:

* **C++ Compiler:** Supporting C++20 standard
  * **Windows:** Visual Studio 2022 (with *Desktop development with C++*)
  * **Linux:** GCC 11+ or Clang 13+
  * **macOS:** Xcode Command Line Tools (Clang)
* **CMake:** Version 3.20 or higher
* **Git**

---

### 2. Installing vcpkg

`vcpkg` is used to fetch and compile third-party libraries automatically during the CMake configuration step.

#### **Windows (PowerShell)**

```powershell
# 1. Clone vcpkg to a persistent directory
git clone https://github.com/microsoft/vcpkg.git C:\microsoft\vcpkg

# 2. Run the bootstrap script to create vcpkg.exe
cd C:\microsoft\vcpkg
.\bootstrap-vcpkg.bat

# 3. Set the environment variable for your current session / system
$env:VCPKG_ROOT="C:\microsoft\vcpkg"
[System.Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\microsoft\vcpkg", [System.EnvironmentVariableTarget]::User)
```

#### **Linux / macOS (Bash / Zsh)**

```bash
# 1. Clone vcpkg
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg

# 2. Run the bootstrap script
cd ~/vcpkg
./bootstrap-vcpkg.sh

# 3. Export VCPKG_ROOT in your shell profile (~/.bashrc or ~/.zshrc)
export VCPKG_ROOT="$HOME/vcpkg"
```

---


Since the project uses `vcpkg.json`, dependencies will automatically download and build during the initial CMake setup.

#### **Step A: Configure CMake**

Pass the `vcpkg` toolchain file during configuration:

* **Linux / macOS:**
  ```bash
  cmake -B build -S . \
    -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release
  ```

* **Windows (PowerShell):**
  ```powershell
  cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
  ```

#### **Step B: Compile the Binary**

```bash
cmake --build build --config Release
```

Once complete, the executable entry point will be output in:
* **Linux/macOS:** `./build/xtrpg_cpp_server`
* **Windows:** `.\build\Release\xtrpg_cpp_server.exe`



## Dockerfile

```shell
# Build the Docker image locally
docker build -t xmpp-builder .

# Extract the compiled binary out of Docker onto the host OS
docker run --rm -v $(pwd)/dist:/output xmpp-builder cp /app/xmpp_server /output/
```

# XTRPG C++ Server

XMPP Server written in C++ built specifically around the XTRPG specifications.

## 🛠️ Project Setup & Installation

This project uses **CMake** (v3.20+) and **vcpkg** in **Manifest Mode** (`vcpkg.json`) to manage dependencies (such as Asio and OpenSSL) across Windows, Linux, and macOS.

---

### Prerequisites

Before building the project, ensure you have the following installed:

* **C++ Compiler:** Supporting C++20 standard
  * **Windows:** Visual Studio 2022 (with *Desktop development with C++*)
  * **Linux:** GCC 11+ or Clang 13+
  * **macOS:** Xcode Command Line Tools (Clang)
* **CMake:** Version 3.20 or higher
* **Git**

---

### Installing vcpkg

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

### Generate Self-Signed SSL Certificates

```bash
# WINDOWS
openssl req -x509 -newkey rsa:2048 -keyout server.key -out server.crt -days 365 -nodes -subj "/CN=localhost"

# LINUX/MACOS
openssl req -x509 -newkey rsa:2048 -keyout server.key -out server.crt -days 365 -nodes -subj "//CN=localhost"
```


---

### Building the Project

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
* **Single-config generators (Linux/macOS):** `./build/xtrpg_cpp_server`
* **Multi-config generators (Windows/Visual Studio):** `.\build\Release\xtrpg_cpp_server.exe`

For multi-config generators on Linux/macOS, use `./build/<Config>/xtrpg_cpp_server` instead.


## Dockerfile

```shell
# Build the Docker image locally
docker build -t xmpp-builder .

# Extract the compiled binary out of Docker onto the host OS
docker run --rm -v $(pwd)/dist:/output xmpp-builder cp /app/xtrpg_cpp_server /output/
```

## External Links

### Core RFCs

- [RFC6120](https://datatracker.ietf.org/doc/html/rfc6120) Extensible Messaging and Presence Protocol (XMPP): Core
- [RFC6121](https://datatracker.ietf.org/doc/html/rfc6121) Extensible Messaging and Presence Protocol (XMPP):
                     Instant Messaging and Presence
- [XEP-0134](https://xmpp.org/extensions/xep-0134.html) XMPP Design Guidelines

### Baseline XEPs

- [XEP-0030](https://xmpp.org/extensions/xep-0030.html) Service Discovery
- [XEP-0115](https://xmpp.org/extensions/xep-0115.html) Entity Capabilities
- [XEP-0198](https://xmpp.org/extensions/xep-0198.html) Stream Management
- [XEP-0280](https://xmpp.org/extensions/xep-0280.html) Message Carbons
- [XEP-0313](https://xmpp.org/extensions/xep-0313.html) Message Archive Management (MAM)
- [XEP-0363](https://xmpp.org/extensions/xep-0363.html) HTTP File Upload

### Federation Minimums (Server-to-Server)

- [XEP-0220](https://xmpp.org/extensions/xep-0220.html) Server Dialback
- [XEP-0368](https://xmpp.org/extensions/xep-0368.html) SRV records for XMPP over TLS

### Group Chat & Social Infrastructure

- [XEP-0045](https://xmpp.org/extensions/xep-0045.html) Multi-User Chat (MUC)
- [XEP-0060](https://xmpp.org/extensions/xep-0060.html) Publish-Subscribe (PubSub)
- [XEP-0163](https://xmpp.org/extensions/xep-0163.html) Personal Event Protocol (PEP)

### Mobile Battery & Network Optimization

- [XEP-0352](https://xmpp.org/extensions/xep-0352.html) Client State Indication (CSI)
- [XEP-0357](https://xmpp.org/extensions/xep-0357.html) Push Notifications
- [XEP-0199](https://xmpp.org/extensions/xep-0199.html) XMPP Ping


### Messaging Experience & Privacy Controls

- [XEP-0085](https://xmpp.org/extensions/xep-0085.html) Chat State Notifications
- [XEP-0184](https://xmpp.org/extensions/xep-0184.html) Message Delivery Receipts
- [XEP-0191](https://xmpp.org/extensions/xep-0191.html) Blocking Command
- [XEP-0203](https://xmpp.org/extensions/xep-0203.html) Delayed Delivery
- [XEP-0333](https://xmpp.org/extensions/xep-0333.html) Chat Markers

### Identity & Server Administration

- [XEP-0004](https://xmpp.org/extensions/xep-0004.html) Data Forms
- [XEP-0077](https://xmpp.org/extensions/xep-0077.html) In-Band Registration
- [XEP-0114](https://xmpp.org/extensions/xep-0114.html) Jabber Component Protocol
- [XEP-0133](https://xmpp.org/extensions/xep-0133.html) Service Administration
- [XEP-0215](https://xmpp.org/extensions/xep-0215.html) External Service Discovery
# tude++ Environment Setup

This guide walks you through setting up a C++ development environment on Windows using WSL2 (Windows Subsystem for Linux). By the end, you will have a Linux environment with a modern C++ compiler, CMake, sanitizers, and an editor connected to WSL2.

**Why Linux?** The delivery platform for your C++ work is Linux. WSL2 gives you a real Linux kernel running on your Windows machine -- not a compatibility layer, but actual Linux. The compilers, sanitizers, and tools you use here are identical to what runs in production. Getting comfortable with Linux now is part of the transition.

**Why not Visual Studio?** Visual Studio is an excellent C# IDE. It also supports C++ on Windows. But MSVC (the Microsoft C++ compiler) has different behavior from GCC and Clang in areas that matter for this curriculum: sanitizer support, warning behavior, and standard conformance edge cases. The cpptudes are tested against GCC and Clang on Linux. Using the same environment eliminates "works on my machine" friction.

---

## Prerequisites

Before starting, you need:

- **Windows 10 (version 2004+) or Windows 11**
- **Administrator access** to your machine
- **~10 GB free disk space** for WSL2 and development tools

You do not need prior Linux experience. This guide explains every command.

---

## Step 1: Install WSL2

WSL2 runs a full Linux virtual machine integrated into Windows. It shares your network and can access your Windows files, but it has its own file system and its own installed programs. Think of it as a second development machine that starts instantly and shares your screen. When you install `g++` inside WSL2, it is not installed on Windows. When you create a file in `~/projects/`, it does not appear in `C:\Users\...`. The two systems coexist but are separate.

> **Two terminals.** From this point forward, there are two terminals on your machine: **PowerShell** (Windows) and **Ubuntu** (Linux/WSL2). Commands beginning with `wsl` run in PowerShell. All other commands in this guide run in the Ubuntu terminal. A program installed in one is not available in the other.

Open **PowerShell as Administrator** and run:

```powershell
wsl --install
```

This installs WSL2 with Ubuntu (the default distribution). If WSL is already installed, you can install Ubuntu explicitly:

```powershell
wsl --install -d Ubuntu-24.04
```

Restart your machine when prompted.

After restart, Ubuntu will launch automatically and ask you to create a username and password. These are your Linux credentials -- they are separate from your Windows login.

### Verify WSL2

Open a new **PowerShell** window and run:

```powershell
wsl --list --verbose
```

You should see Ubuntu listed with VERSION 2. If it shows VERSION 1, upgrade it:

```powershell
wsl --set-version Ubuntu-24.04 2
```

---

## Step 2: Update Ubuntu and Install C++ Toolchain

Open your **Ubuntu terminal** (search "Ubuntu" in the Start menu) and run:

```bash
sudo apt update && sudo apt upgrade -y
```

> **What is `sudo`?** The `sudo` command runs a single command with administrator privileges -- similar to right-clicking "Run as administrator" in Windows. It will prompt for the Linux password you created during WSL2 setup.

> **What is `apt`?** `apt` is Ubuntu's package manager -- think of it as NuGet for the entire operating system. `apt update` refreshes the package list, `apt install` installs packages.

Then install the C++ development toolchain:

```bash
sudo apt install -y \
    build-essential \
    gcc-13 \
    g++-13 \
    clang-17 \
    clang-tidy-17 \
    cmake \
    ninja-build \
    gdb \
    git \
    curl \
    pkg-config
```

> The `\` at the end of a line means the command continues on the next line. You can also type this as a single long line.

### Set GCC 13 and Clang 17 as defaults

```bash
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-17 100
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-17 100
```

### Verify installations

```bash
g++ --version        # Should show 13.x
clang++ --version    # Should show 17.x
cmake --version      # Should show 3.20+
ninja --version      # Should show 1.10+
gdb --version        # Should show 12+
git --version        # Should show 2.x
```

If `g++-13` or `clang-17` are not available in your Ubuntu version's default repositories, add the toolchain PPA:

```bash
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo apt update
sudo apt install -y gcc-13 g++-13
```

For Clang 17:

```bash
# Add LLVM repository (copy and paste this block exactly)
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 17
rm llvm.sh
```

---

## Step 3: Configure Git

You likely already have Git configured on Windows. WSL2 is a separate environment and needs its own Git configuration.

```bash
git config --global user.name "Your Name"
git config --global user.email "your.email@example.com"
git config --global init.defaultBranch main
git config --global core.autocrlf input
```

The `core.autocrlf input` setting converts Windows-style line endings (CRLF) to Linux-style (LF) on commit. The cpptudes repository also includes a `.gitattributes` file that enforces LF for all source files.

### SSH keys

If you use SSH to access GitHub, you need a key inside WSL2. Either generate a new one:

```bash
ssh-keygen -t ed25519 -C "your.email@example.com"
cat ~/.ssh/id_ed25519.pub
```

Then add the public key to your GitHub account at https://github.com/settings/keys.

Or, if you prefer to reuse your Windows SSH key, you can copy it:

```bash
cp /mnt/c/Users/YOUR_WINDOWS_USERNAME/.ssh/id_ed25519 ~/.ssh/
cp /mnt/c/Users/YOUR_WINDOWS_USERNAME/.ssh/id_ed25519.pub ~/.ssh/
chmod 600 ~/.ssh/id_ed25519
chmod 644 ~/.ssh/id_ed25519.pub
```

> The `chmod` numbers set file permissions: `600` means only you can read and write the file. SSH requires this for security.

### Alternative: HTTPS with GitHub CLI

If you prefer HTTPS over SSH (or if your organization blocks SSH), install the GitHub CLI:

```bash
sudo apt install -y gh
gh auth login
```

Follow the prompts to authenticate. This handles credentials without SSH keys.

### Verify GitHub access

```bash
ssh -T git@github.com       # If using SSH
# or
gh auth status               # If using GitHub CLI
```

### Daily Git workflow

If you have been using Visual Studio's Git integration or GitHub Desktop, here are the terminal equivalents:

```bash
git status                 # See what changed (like the Changes panel in VS)
git add file.cpp           # Stage a file for commit
git add .                  # Stage all changes
git commit -m "message"    # Commit (like the commit textbox + button in VS)
git push                   # Push to remote (like the push/sync button in VS)
git pull                   # Pull latest changes
git log --oneline -10      # View recent history
```

---

## Step 4: Set Up Your Editor

### Option A: VS Code with WSL2 (Recommended)

If you already use VS Code for C# development, this is the smoothest path. VS Code connects to WSL2 natively.

1. Install [VS Code](https://code.visualstudio.com/) on Windows (if not already installed).
2. Install the **WSL** extension (by Microsoft) from the Extensions panel.
3. Open your Ubuntu terminal and navigate to your project directory:

```bash
cd ~
git clone <your-cpptudes-repo-url>
cd cpptudes
code .
```

This opens VS Code connected to WSL2. The terminal inside VS Code is your Linux shell. File operations, compilation, and debugging all happen inside Linux.

4. Install these VS Code extensions (they install inside WSL2 automatically):
   - **C/C++** (by Microsoft) -- IntelliSense, debugging, formatting
   - **CMake Tools** (by Microsoft) -- CMake integration, build targets sidebar (replaces Solution Explorer)
   - **clangd** (by LLVM) -- alternative to Microsoft C/C++ for faster IntelliSense (optional, pick one)

> **IntelliSense setup.** For IntelliSense to find headers and flags correctly, CMake needs to generate a `compile_commands.json` file. Add `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON` to your CMake configure command, or let CMake Tools handle it automatically (it does this by default).

> **Debugging.** The C/C++ extension provides the same click-to-set-breakpoint and F5-to-debug experience you know from Visual Studio. The first time you press F5, VS Code will prompt you to create a `launch.json` configuration. Select "C++ (GDB/LLDB)" and point it at your compiled executable. Breakpoints, watch windows, and call stacks all work as expected.

### Option B: CLion

JetBrains CLion has native WSL2 support. If you have a JetBrains license:

1. Install CLion on Windows.
2. Go to **Settings > Build, Execution, Deployment > Toolchains**.
3. Add a WSL toolchain. CLion will auto-detect GCC, CMake, and GDB inside your Ubuntu installation.

### Option C: Terminal-only (Vim/Neovim)

This option is listed for completeness. If you are coming from Visual Studio, Option A (VS Code) is the recommended path.

```bash
sudo apt install -y neovim
```

No further configuration is needed for the cpptudes. The build system is CMake, and all commands run from the terminal.

---

## Step 5: Build and Test the Cpptudes

Clone the repository (if you have not already):

```bash
cd ~
git clone <your-cpptudes-repo-url>
cd cpptudes
```

### Build Cpptude #000 (Setup Guide)

```bash
cd cpptudes/000-setup-guide
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Run it:

```bash
./build/setup-guide
```

You should see sanitizer output catching deliberate bugs. This confirms your compiler, CMake, and sanitizers are all working.

### Build Cpptude #001 (Sudoku)

```bash
cd ../001-sudoku
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/sudoku
```

If both cpptudes build and run, your environment is ready.

---

## Step 6: Understand the Linux File System

Coming from Windows, the file system layout is different. Here is what you need to know:

| Concept | Windows | Linux (WSL2) |
|---------|---------|--------------|
| Home directory | `C:\Users\YourName` | `/home/yourname` (or `~`) |
| Project location | Wherever you like | Use `~/projects/` or similar under your home directory |
| Windows files from Linux | N/A | `/mnt/c/Users/YourName/...` |
| Linux files from Windows | N/A | `\\wsl$\Ubuntu\home\yourname\...` (type in Explorer address bar) |
| Path separator | `\` | `/` |
| Case sensitivity | Case-insensitive | **Case-sensitive** -- `File.cpp` and `file.cpp` are different files |
| Line endings | CRLF (`\r\n`) | LF (`\n`) |
| Executable permission | By extension (`.exe`) | By permission flag (`chmod +x`) |

### Important: Keep projects inside WSL2

Store your cpptudes project inside the Linux file system (`~/projects/cpptudes`), not on the Windows mount (`/mnt/c/...`). The Windows mount has significantly slower I/O and does not support Linux file permissions correctly. Compilation will be noticeably faster inside the native Linux file system.

### Case sensitivity warning

If you rename a file changing only its case (e.g., `Solver.cpp` to `solver.cpp`), Linux treats these as different files. Use `git mv OldName.cpp newname.cpp` to rename safely.

### No NuGet

The cpptudes have no external library dependencies beyond the C++ standard library. If you are wondering "where is NuGet?" -- C++ does not have a single dominant package manager. The cpptudes use only standard library features and system-installed tools.

---

## Step 7: Terminal Basics for C# Developers

If you have been using Visual Studio and PowerShell, the Linux terminal will feel different. Here are the essential commands:

> Linux commands pipe text between each other with `|`, similar to PowerShell pipelines -- but Linux pipes raw text, not objects.

### Navigation

```bash
pwd                    # Print working directory (like `Get-Location` in PowerShell)
ls                     # List files (like `dir` or `Get-ChildItem`)
ls -la                 # List all files with details (permissions, size, dates)
cd ~/projects          # Change directory (like `Set-Location`)
cd ..                  # Go up one directory
cd -                   # Go back to previous directory
which g++              # Find where a command is installed (like `Get-Command`)
```

### Files

```bash
cat file.txt           # Print file contents (like `Get-Content`)
less file.txt          # View file with scrolling (arrows to scroll, / to search, q to quit)
grep -r "pattern" .    # Search file contents recursively (like `Select-String`)
cp source dest         # Copy file (like `Copy-Item`)
mv source dest         # Move or rename file (like `Move-Item`)
rm file.txt            # Delete file (like `Remove-Item`) -- no recycle bin
mkdir -p path/to/dir   # Create directory and parents (like `New-Item -Type Directory`)
clear                  # Clear the screen (like `cls`)
```

### Build commands you will use

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug   # Configure with CMake (Debug enables sanitizers)
cmake --build build                        # Build the project
./build/program_name                       # Run the compiled program (no .exe extension)
```

### Process control

```bash
Ctrl+C                 # Stop a running program (like clicking Stop in Visual Studio)
Ctrl+D                 # Close the terminal session (like typing `exit`)
```

### Getting help

```bash
man command            # Read the manual page (arrows to scroll, / to search, q to quit)
command --help         # Most commands have a --help flag
```

---

## Troubleshooting

### "Command not found: g++-13"

Your Ubuntu version may not include GCC 13 in the default repositories. Add the toolchain PPA:

```bash
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo apt update && sudo apt install -y gcc-13 g++-13
```

### "CMake version too old"

Ubuntu 22.04 ships CMake 3.22, which is sufficient (3.20+ required). If you have an older version:

```bash
sudo apt install -y cmake
cmake --version
```

If still too old, install from Kitware's repository (copy and paste this block exactly -- you do not need to understand each part):

```bash
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | \
    gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" | \
    sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null
sudo apt update && sudo apt install -y cmake
```

### "Cannot connect to GitHub via SSH from WSL2"

Make sure your SSH key has the correct permissions:

```bash
chmod 700 ~/.ssh
chmod 600 ~/.ssh/id_ed25519
chmod 644 ~/.ssh/id_ed25519.pub
```

Then test:

```bash
ssh -T git@github.com
```

### "VS Code doesn't show IntelliSense / red squiggles"

Make sure you have opened the project from inside WSL2 (`code .` from the Ubuntu terminal), not from Windows. Check the bottom-left corner of VS Code -- it should show "WSL: Ubuntu" in the green status bar.

If IntelliSense is not finding headers, ensure `compile_commands.json` exists in your build directory. Reconfigure with:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

Then point your C/C++ extension or clangd at the generated `build/compile_commands.json`.

### "Sanitizer reports 'LeakSanitizer does not work under ptrace'"

This happens when running under certain debuggers. Two options:

1. Run the program outside the debugger to see sanitizer output.
2. Adjust the ptrace scope (this also fixes GDB attach issues):

```bash
echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope
```

Or, to disable leak detection only (preserving other sanitizer checks):

```bash
export ASAN_OPTIONS=detect_leaks=0
```

### WSL2 is using too much memory

WSL2 defaults to using up to 50% of your system RAM. To limit it, create or edit `%UserProfile%\.wslconfig` on Windows:

```ini
[wsl2]
memory=4GB
swap=2GB
```

Then restart WSL2:

```powershell
wsl --shutdown
```

---

## What You Have Now

After completing this guide, your environment includes:

| Tool | Purpose | Version |
|------|---------|---------|
| **Ubuntu on WSL2** | Linux environment for C++ development | 24.04 LTS |
| **GCC 13** | Primary C++ compiler | 13.x |
| **Clang 17** | Secondary compiler, clang-tidy for static analysis | 17.x |
| **CMake** | Build system generator | 3.20+ |
| **Ninja** | Fast build executor (used by CMake) | 1.10+ |
| **GDB** | Debugger | 12+ |
| **ASan/UBSan** | Sanitizers (included with GCC and Clang) | Built-in |
| **Git** | Version control | 2.x |
| **VS Code + WSL** | Editor connected to Linux environment | Latest |

This matches the toolchain specified in the cpptudes. Every cpptude's CMakeLists.txt is configured to use these tools.

> **A note on `#include` vs `using`.** You will notice C++ uses `#include` rather than C#'s `using`. How this works -- and how it differs from C# project references -- is covered in Cpptude #002 (File Processor), which introduces the C++ compilation model.

---

## Next Step

Open Cpptude #000: Setup Guide. It will verify that your sanitizers, compiler warnings, and build system are working correctly. If #000 passes, you are ready for the curriculum.

Below is a **step-by-step guide** for **building and installing LLVM/Clang** on **Windows** using **Ninja** as the build tool. You will still be using Microsoft’s C++ toolchain (from Visual Studio) but telling CMake to generate Ninja build files instead of Visual Studio projects.

---

## 1. Prerequisites

1. **Visual Studio** (or the Visual Studio Build Tools)  
   - Make sure the **Desktop development with C++** workload is installed.  
   - You will typically use the **x64 Native Tools Command Prompt** (or a Developer Command Prompt) that configures your environment for the MSVC compiler.

2. **Git**  
   - Required to clone the repository.

3. **CMake**  
   - Make sure `cmake` is available in your `PATH`.

4. **Ninja**  
   - Download and install [Ninja](https://ninja-build.org/), and ensure `ninja.exe` is accessible from the command line (in your `PATH`).

---

## 2. Get the Source

Open the **x64 Native Tools Command Prompt** for your version of Visual Studio. Then:

```bash
git clone https://github.com/llvm/llvm-project.git
```

This will create a folder named `llvm-project`.

---

## 3. Create a Build Directory

1. Move into the cloned repository folder:
   ```bash
   cd llvm-project
   ```
2. Create a separate build folder (to keep build files separate from source):
   ```bash
   mkdir build
   cd build
   ```

---

## 4. Configure CMake with Ninja

Still in the `build` folder, run:

```bash
cmake ^
  -G "Ninja" ^
  -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" ^
  -DLLVM_TARGETS_TO_BUILD="X86" ^
  -DLLVM_ENABLE_RTTI=ON ^
  -DCMAKE_BUILD_TYPE=Release ^
  ..\llvm
```

### Explanation of Key Options

- **-G "Ninja"**: Use Ninja as the generator (instead of Visual Studio project files).  
- **-DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra"**: Build Clang and the Clang extra tools.  
- **-DLLVM_TARGETS_TO_BUILD="X86"**: Only build the X86 backend (you can build more if you need them).  
- **-DLLVM_ENABLE_RTTI=ON**: Enable Run-Time Type Information; needed by some use cases.  
- **-DCMAKE_BUILD_TYPE=Release**: Build in Release mode (other options include `Debug`, `RelWithDebInfo`, etc.).  

> **Optional**: If you want to install LLVM in a custom path (e.g. `C:\llvm-install`), add:
> ```bash
> -DCMAKE_INSTALL_PREFIX="C:\llvm-install"
> ```
> to the command above.

---

## 5. Build with Ninja

Once CMake has finished configuring, you’ll have Ninja build files in your `build` directory.

To build:

```bash
ninja
```

> **Tip**: You can speed up builds on multi-core machines with:
> ```bash
> ninja -j 8
> ```
> (where `8` is the number of parallel jobs—adjust to the number of CPU cores you have).

---

## 6. Install

After the build succeeds, install LLVM/Clang to the chosen `CMAKE_INSTALL_PREFIX` (or to the default location if none was specified):

```bash
ninja install
```

- If you **did not** specify `-DCMAKE_INSTALL_PREFIX`, the default install directory is typically:
  ```
  C:\Program Files (x86)\LLVM
  ```

---

## 7. Verify the Installation

1. If you **did** specify a custom install prefix like `C:\llvm-install`, go check:
   ```
   C:\llvm-install\bin
   ```
   You should see `clang.exe`, `clang++.exe`, `clang-tidy.exe`, etc.
2. If you **did not** specify one, check:
   ```
   C:\Program Files (x86)\LLVM\bin
   ```
3. Add that `bin` folder to your system `PATH` if you want to run `clang` from any command prompt.

With that, you have a working **LLVM + Clang** build on Windows using Ninja!

## 8. Configure CMake for the code

```bash
mkdir build
cd build
cmake ..
```

## 9. Execute code
```bash
ninja
```

```bash
.\ASTTraverser.exe .\input.cpp  -- -std=c++17
```
<div align="center">

# hookhound

**Detect userland hooks and code tampering in running Windows processes.**

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-build-064F8C?style=flat-square&logo=cmake&logoColor=white)
![Windows 11 ARM64](https://img.shields.io/badge/Windows%2011-ARM64-0078D4?style=flat-square&logo=windows11&logoColor=white)
![Status](https://img.shields.io/badge/status-v0.1%20in%20progress-orange?style=flat-square)

</div>

---

hookhound is a read-only command-line tool that compares the code a process has loaded in memory against the original files on disk. Any difference that the Windows loader doesn't explain is reported as a finding, with the module, location, and bytes involved.

## How it works

When Windows loads a DLL such as `ntdll.dll`, it copies the file from disk into the process's memory. The executable code in memory should match the code on disk, apart from addresses the loader legitimately adjusts (relocations). If the bytes differ in any other way, something modified them.

hookhound:

1. Parses the PE (Portable Executable) file on disk.
2. Reads the same module out of the target process's memory.
3. Normalizes for relocations, so legitimate differences are ignored.
4. Compares the executable, non-writable sections (mainly `.text`).
5. Reports whatever is left, and classifies it by hook type.

> [!TIP]
> Think of it as comparing a photocopy of a contract to the original: if a clause changed, someone tampered with it.

## Detections

| Detection | What it catches |
|---|---|
| **Inline hooks** | Modified function prologues that redirect execution (ARM64: `B`, `BL`, `BR`, `BLR`, `LDR`+`BR`, `ADRP`+`ADD`+`BR`; x64 planned) |
| **IAT hooks** | Import Address Table entries pointing outside the module that should provide the function |
| **EAT hooks** | Export Address Table entries that differ from the on-disk export table |
| **Unbacked executable memory** | Executable regions not backed by a mapped image file (private or mapped, not `MEM_IMAGE`) |

Where possible, findings include the destination of a hook and which module (or unbacked region) it lands in.

## Design principles

- **Read-only.** hookhound never writes to, injects into, or modifies another process.
- **Trustworthy output.** Few false positives; every finding says what changed and where.
- **Scriptable.** Human-readable output by default, JSON for tooling, meaningful exit codes.
- **Defensive handling of input.** Every offset, size, and count in a PE file is treated as untrusted. Malformed files produce a clear error, never a crash or hang.

## Platform support

| Target | Status |
|---|---|
| Windows 11 ARM64 (native) | ![Primary target](https://img.shields.io/badge/primary%20target-2ea44f?style=flat-square) |
| Windows x64 | ![Planned](https://img.shields.io/badge/planned-blue?style=flat-square) |
| Linux / macOS targets | ![Out of scope](https://img.shields.io/badge/out%20of%20scope-lightgrey?style=flat-square) |

> [!IMPORTANT]
> hookhound should be run as a **native ARM64** build on Windows ARM. x64 programs on Windows ARM run under an emulation layer that patches memory and loads extra DLLs, which can look like hooks. Emulated x64 targets are flagged or skipped with a notice.

> [!NOTE]
> Windows 11 ARM system DLLs may carry ARM64EC/ARM64X metadata, so some data directories in files like `ntdll.dll` can look unusual. Start with simpler binaries such as `notepad.exe` when testing.

## Architecture

The code is split so most of it builds and tests on macOS.

```
hookhound/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp        CLI entry point and argument parsing
│   ├── core/           Portable: PE parsing, relocations, comparison, detection, reporting
│   └── win/            Windows only: process access, module listing, memory reads
├── tests/
└── testdata/           Real Windows files for local testing (not committed)
```

- `src/core/` works only on byte buffers and includes no Windows headers. PE structures and constants are defined in the project.
- `src/win/` contains every Windows API call and implements an abstract *process memory reader* (read bytes, list modules, list regions).
- **Tests** use a mock reader that serves bytes from patched copies of real files.
- Detection produces structured findings; the text and JSON reporters both consume the same data.

## Building

**Requirements:** a C++17 compiler and CMake.

```bash
cmake -S . -B build
cmake --build build
```

To build only the portable core and tests (for example on macOS):

```bash
cmake -S . -B build -DHOOKHOUND_CORE_ONLY=ON
cmake --build build
```

Warnings are enabled (`-Wall -Wextra`, `/W4` on MSVC). On macOS, run the tests with AddressSanitizer and UndefinedBehaviorSanitizer enabled.

## Usage

Planned commands, by version:

```bash
hookhound pe <file>                 # v0.1  print the structure of a PE file
hookhound self                      # v0.2  compare memory vs. disk in hookhound's own process
hookhound scan --pid <pid>          # v0.3  scan another process by PID
hookhound scan --name <name>        # v0.3  scan another process by name
```

**Planned flags (v0.5):** `--json`, `--watch <seconds>`, `--quiet`, `--output <file>`, `--help`, `--version`.

**Planned exit codes:** `0` clean, `1` findings, `2` error.

Real usage examples and output samples will be added as each command lands.

## Roadmap

| Version | Goal | Status |
|---|---|---|
| **v0.1** | PE parser: headers, sections, imports, exports, RVA conversion | ![In progress](https://img.shields.io/badge/in%20progress-orange?style=flat-square) |
| **v0.2** | Memory vs. disk comparison in the current process, with relocation normalization | ![Planned](https://img.shields.io/badge/planned-blue?style=flat-square) |
| **v0.3** | Scan other processes through the process reader interface | ![Planned](https://img.shields.io/badge/planned-blue?style=flat-square) |
| **v0.4** | Typed findings: inline, IAT, EAT hooks, and unbacked executable memory | ![Planned](https://img.shields.io/badge/planned-blue?style=flat-square) |
| **v0.5** | JSON output, `--watch`, exit codes, polish, license | ![Planned](https://img.shields.io/badge/planned-blue?style=flat-square) |

> [!NOTE]
> **v0.1 is done when** its output matches `dumpbin /headers /imports /exports` on `notepad.exe`, `kernel32.dll`, `kernelbase.dll`, `user32.dll`, and `ntdll.dll`; truncated and corrupted files are rejected cleanly; and sanitizer builds are clean.

## Testing

- **Unit tests (macOS or Windows):** relocation handling, comparison, and detection logic run against synthetic or patched PE files generated in code.
- **Patched-copy fixtures:** real PE files with a few bytes changed simulate inline, IAT, and EAT hooks.
- **Self-patching program (Windows VM):** a small program overwrites the first bytes of one of its own functions with a branch and waits, giving a reliable true positive.
- **Clean baseline:** a fresh VM snapshot with no third-party software, so "clean" means clean.

> [!CAUTION]
> Real Windows binaries used for local testing live in `testdata/` and are **not committed**, since they are Microsoft's copyrighted files.

## Safety and scope

> [!WARNING]
> - hookhound only scans systems and processes you control.
> - Analyze malware samples without executing them, and never on a VM with shared folders, clipboard sharing, or networking enabled. Snapshot the VM before risky tests.

**Out of scope:** kernel drivers and kernel-mode hook detection, removing or modifying hooks, injecting into processes, Linux and macOS targets, and a GUI.

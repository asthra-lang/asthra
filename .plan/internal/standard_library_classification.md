
# 📘 Reference Report: Hierarchical Classification of Standard Libraries

This report presents a **hierarchical classification** of standard libraries inspired by popular languages such as **Python** and **Go**. It organizes libraries by both their **high-level purpose** (Essential, Intermediate, Advanced) and **functional category** for easier understanding, especially by non-technical stakeholders.

---

## 🔹 1. Essential (Core/Foundation)

These libraries form the foundation of any programming language and are critical for everyday development tasks.

### ▶️ Core Language Support
**Purpose:** Access to runtime internals, system calls, and built-in language features.  
- **Python:** `sys`, `builtins`, `types`, `platform`  
- **Go:** `runtime`, `unsafe`, `syscall`

### ▶️ File and OS Interaction
**Purpose:** Filesystem navigation, file I/O, and process/environment handling.  
- **Python:** `os`, `shutil`, `pathlib`, `tempfile`  
- **Go:** `os`, `io/ioutil`, `filepath`

### ▶️ Text and String Processing
**Purpose:** String manipulation, regex, and templating.  
- **Python:** `re`, `string`, `textwrap`, `difflib`  
- **Go:** `strings`, `regexp`, `unicode`, `text/template`

### ▶️ Data Structures and Algorithms
**Purpose:** Common data containers and search/sort algorithms.  
- **Python:** `collections`, `heapq`, `bisect`, `array`  
- **Go:** `container/heap`, `container/list`, `sort`, `bytes`

### ▶️ Date and Time
**Purpose:** Working with timestamps, durations, and calendars.  
- **Python:** `datetime`, `time`, `calendar`  
- **Go:** `time`

### ▶️ Math and Statistics
**Purpose:** Math functions, statistics, and numerical computation.  
- **Python:** `math`, `cmath`, `statistics`, `decimal`  
- **Go:** `math`, `math/big`, `math/rand`, `math/cmplx`

---

## 🔸 2. Intermediate (Application-Level)

These libraries are necessary for building real-world applications that involve networking, parallelism, and data communication.

### ▶️ Networking and Web
**Purpose:** Develop networked and web applications.  
- **Python:** `socket`, `http.client`, `urllib`, `http.server`  
- **Go:** `net`, `net/http`, `net/url`, `net/rpc`

### ▶️ Concurrency and Parallelism
**Purpose:** Support for multi-threading, async, and parallel execution.  
- **Python:** `threading`, `multiprocessing`, `asyncio`  
- **Go:** Goroutines, `sync`, `context`

### ▶️ Serialization and Parsing
**Purpose:** Reading and writing structured data formats.  
- **Python:** `json`, `pickle`, `csv`, `xml.etree.ElementTree`  
- **Go:** `encoding/json`, `encoding/xml`, `encoding/csv`, `encoding/gob`

### ▶️ Testing and Debugging
**Purpose:** Writing tests and inspecting code behavior.  
- **Python:** `unittest`, `doctest`, `pdb`, `trace`  
- **Go:** `testing`, `testing/quick`, `pprof`

---

## 🔺 3. Advanced (Expert/Power User Tools)

Used in specialized domains such as security, tooling, compiler construction, and performance optimization.

### ▶️ Cryptography and Security
**Purpose:** Secure hashing, encryption, and SSL/TLS.  
- **Python:** `hashlib`, `hmac`, `ssl`, `secrets`  
- **Go:** `crypto/*`, `tls`, `x/crypto`

### ▶️ Development and Reflection
**Purpose:** Compiler tooling, AST parsing, and runtime introspection.  
- **Python:** `inspect`, `ast`, `dis`, `typing`  
- **Go:** `reflect`, `go/ast`, `go/parser`, `go/types`

---

## ✅ Summary Table

| Level          | Goal / Audience                         | Included Categories                                               |
|----------------|------------------------------------------|-------------------------------------------------------------------|
| **Essential**  | All developers, beginners, core usage    | Core, File/OS, Text, Data, Date/Time, Math                        |
| **Intermediate** | Application developers                  | Networking, Concurrency, Serialization, Testing                   |
| **Advanced**    | Experts, systems/tooling/security devs  | Cryptography, Development and Reflection                          |

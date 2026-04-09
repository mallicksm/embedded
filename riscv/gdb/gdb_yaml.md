# GDB + PyYAML Setup (Minimal Guide)
This project uses GDB Python scripts that depend on **PyYAML**.
GDB uses its own Python environment, so you must explicitly bridge it.

### 1. Create and activate a virtual environment

```bash
python3 -m venv venv
source venv/bin/activate
```
### 2. Install PyYAML into the venv

```bash
python -m pip install pyyaml
```
### Check search paths

```gdb
python import sys; print(sys.path)
```
## 💡 Mental model

```text
pip install → puts package somewhere
GDB → cannot see it
sys.path → tells GDB where to look
```

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
### 3. Verify installation

```bash
python -c "import yaml; print(yaml.__file__)"
```
Example output:
```text
/Users/mallicksm/venv/lib/python3.11/site-packages/yaml/__init__.py
```
## 🔧 GDB setup

Edit `debug.gdb` and add:

```gdb
python
import sys
sys.path.insert(0, "/Users/mallicksm/venv/lib/python3.11/site-packages")
end
```
## 🧪 Verify inside GDB

```gdb
python import yaml; print("yaml OK")
```
### Check GDB Python path

```gdb
python import sys; print(sys.executable)
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

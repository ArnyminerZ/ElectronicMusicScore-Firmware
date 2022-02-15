import subprocess
import sys
import os

def install(package):
    subprocess.check_call([sys.executable, "-m", "pip", "install", package, "--quiet"])

print("📦 Installing dependencies...")

with open('requirements.txt', 'r') as file:
    lines = file.readlines()
    for line in lines:
        line = line.replace('\n', '')
        print(f"  🔁 Installing {line}...", end='')
        install(line)
        print(f" ✅")

print("✅ Finished installing dependencies.")

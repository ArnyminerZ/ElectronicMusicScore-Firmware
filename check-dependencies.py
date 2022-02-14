import os
import subprocess
import sys

print("Checking if MX is available...")

mx_dir = "./include/mx"
buildmx_dir = './dependencies/buildmx.sh'

if not os.path.exists(buildmx_dir):
    print("⚠️ buildmx dir doesn't exist.")
    sys.exit()

if not os.path.exists(mx_dir):
    print("❌ MX not available. Building...")
    subprocess.call(['sh', buildmx_dir])
    exec(open("./fix-mx-paths.py").read())
else:
    print("✅ MX is available")

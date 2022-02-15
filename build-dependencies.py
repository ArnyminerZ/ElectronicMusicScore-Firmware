import os
import subprocess
import sys

print("🔁 Checking if MX is available...")

mx_dir = "./libs/mx/libmx.a"
buildmx_script = './dependencies/buildmx.sh'

if not os.path.exists(buildmx_script):
    print("⚠️ buildmx script doesn't exist.")
    sys.exit()

if not os.path.exists(mx_dir):
    print("❌ MX not available. Building...")
    # Currently disabled since using Sourcecode files
    # subprocess.call(['sh', buildmx_script])
    # exec(open("./fix-mx-paths.py").read())
else:
    print("✅ MX is available")

import os

mx_path = './include/mx'

files_list = os.listdir(mx_path)

print("🔃 Fixing compatibility for header files...")

for file in files_list:
    print(f"  📝 Patching {file}...", end = '')

    # Read the file data
    with open(f"{mx_path}/{file}", 'r') as rstream:
        filedata = rstream.read()

    # Replace all the desired strings
    filedata = filedata.replace('mx/api/', '')
    filedata = filedata.replace('mx/ezxml/', '')
    filedata = filedata.replace('mx/core/elements/', '')
    filedata = filedata.replace('mx/core/', '')
    filedata = filedata.replace('mx/impl/', '')
    filedata = filedata.replace('mx/utility/', '')
    filedata = filedata.replace('ezxml/', '')

    # Write the changes
    with open(f"{mx_path}/{file}", 'w') as wstream:
        wstream.write(filedata)

    print("ok")

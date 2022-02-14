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
    # This is just required with CPP<17
    # filedata = filedata.replace('<optional>', '<experimental/optional>')
    # filedata = filedata.replace('std::optional', 'std::experimental::optional')
    filedata = filedata.replace('mx/api/', '')

    # Write the changes
    with open(f"{mx_path}/{file}", 'w') as wstream:
        wstream.write(filedata)

    print("ok")

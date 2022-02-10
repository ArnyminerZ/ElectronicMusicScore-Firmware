import os
import htmlmin
import rcssmin
import jsmin

path = r"./www"
target_h = "./include/webpages.h"

files_list = os.listdir(path)

if os.path.exists(target_h):
    os.unlink(target_h)

f = open(target_h, "x")
f.write("/**\n")
f.write(" * @file webpages.h\n")
f.write(" * @author Arnau Mora (arnyminer.z@gmail.com)\n")
f.write(" * @brief This page was automatically generated from the source files at www\n")
f.write(" * \n")
f.write(" * @copyright Copyright (c) 2022\n")
f.write(" * \n")
f.write(" */\n")
f.write("\n")
f.write("#ifndef WEBPAGES_H\n")
f.write("#define WEBPAGES_H\n")
f.write("#include <Arduino.h>\n")

for file in files_list:
    stream = open(f"{path}/{file}", "r")
    filename = file.replace(".", "_")

    f.write(f"const char {filename}[] PROGMEM = R\"rawliteral(\n")
    if file.endswith("html"):
        minified = htmlmin.minify(stream.read())
        f.write(minified)
        f.write("\n")
    elif file.endswith("css"):
        minified = rcssmin.cssmin(stream.read()).replace("%", "%%")
        f.write(minified)
        f.write("\n")
    elif file.endswith("js"):
        minified = jsmin.jsmin(stream.read())
        f.write(minified)
        f.write("\n")
    f.write(")rawliteral\";\n")

    stream.close()

f.write("#endif")
f.close()

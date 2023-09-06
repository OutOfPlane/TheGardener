Import("env")
import os

# General options that are passed to the C and C++ compilers
buildNum = 0

if(os.path.exists("build")):
    f = open("build", "r")
    buildNum = int(f.readline())
    f.close()

buildNum += 1
f = open("build", "w")
f.write(str(buildNum) + "\r\n")
f.close()

f = open("components/buildnumber/include/buildnumber.hpp", "w")
f.write("#ifndef G_VERSION_H\r\n" \
"#define G_VERSION_H\r\n" \
f"#define G_CODE_BUILD {buildNum}\r\n" \
"\r\n"\
"#endif")

f.close()
# env.Append(CCFLAGS=[f"-D PP_CODE_BUILD={buildNum}"])
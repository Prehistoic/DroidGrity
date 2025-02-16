# TO DO

- Fill README.md
- Run tests ! (different APKs, different arch...)

# Possible further enhancements

- Allowing the user to define a handler in its code rather than crashing the application
- Use o-llvm to obfuscate the dylib

# Known pitfalls

- Just patching the application with APKTool by removing the few lines used to call the integrity check JNI method

    - Can be made more difficult to do by using obfuscation to make it less obvious which lines should be removed
    - Even better use a packer so that patching would require patching an encrypted dex which will involve to fully reverse the packing mechanism

- Frida can be used to hook native methods and replace them so basically just have to replace the integrity check JNI method with a log

    - Implement additional anti-frida/hooking tools checks into the application
    - Obfuscating the native code to make it more difficult to know which native method to hook
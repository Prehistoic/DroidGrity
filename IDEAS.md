# TO DO

- Finir debug hash cert quand signing scheme = v1 => remains to get the signature from the PKCS7 Signed DER encoded certificate in memory ! Need to find the same digests than apksigner !
- Finir debug hash cert quand signing scheme = v2 & arch = x86_64

- Clean out the .cpp.template code by dividing it into smaller files (see notes in the template)
- Use o-llvm to obfuscate the dylib

- Fill README.md

- Create a docker image

- Run tests ! (different APKs, different arch...)

# Possible further enhancements

- Make it so the user can define a handler in its code rather than crashing the application. Maybe you could also just start the dylib code with JNI_OnLoad
- Use o-llvm to obfuscate your library when building it ?

# Known pitfalls

- Just patching the application with APKTool by removing the few lines used to call the integrity check JNI method

    - Can be made more difficult to do by using obfuscation to make it less obvious which lines should be removed
    - Even better use a packer so that patching would require patching an encrypted dex which will involve to fully reverse the packing mechanism

- Frida can be used to hook native methods and replace them so basically just have to replace the integrity check JNI method with a log

    - Implement additional anti-frida/hooking tools checks into the application
    - Obfuscating the native code to make it more difficult to know which native method to hook
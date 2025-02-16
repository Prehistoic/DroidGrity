# DroidGrity 🛡️

**DroidGrity** is a tool aiming at protecting APK files against unauthorized tampering. This ensures the Android application can't be cloned and modified to add malicious code or to disable security features.

> ## Disclaimer 📢
>
> **DroidGrity** was created **for educational purposes only**. This project should **not** be used in production applications or in any context requiring reliable security. No guarantees are provided regarding the effectiveness of this software against real-world attacks. 
> 
> To securely protect your Android applications, it is highly recommended to use **professionally developed solutions that have been validated by third-party security audits**.
> 
> Using **DroidGrity** is at your own risk. The author of this project cannot be held responsible for any consequences arising from its use.  

## Features ✨

- **Native Signature check**: 🔍 Integrity check based on signature verification performed in native C++ code
- **Signature Schemes support**: ⚙️ Supports all signature schemes (v1 to v4)
- **Custom libc**: 🛠️ Uses a custom libc to protect against libc hooking 

## Prerequisites 🖥️

- 🐍 Python 3.x
- ☕ Java Development Kit (JDK) > 11
- 🔑 Keystore containing the Application Signing key used to sign the original APK
- 🔧 [APKtool](https://github.com/iBotPeaches/Apktool)
- 🔧 [Android SDK Build Tools](https://developer.android.com/tools?hl=fr#tools-build) *for apksigner & zipalign*
- 🔧 [Android SDK Platform Tools](https://developer.android.com/tools?hl=fr#tools-platform) *for adb*
- 🔧 [cmake](https://cmake.org/)
- 🔨 [Android NDK](https://developer.android.com/ndk/downloads?hl=fr)

> All 🔧 tools must be in PATH
>
> Path to 🔨 Android NDK can either be passed via a CLI parameter or with the *ANDROID_NDK_ROOT* ENV variable

## Installation 🚀

1. **Clone the repository**:

   ```bash
   git clone https://github.com/Prehistoic/DroidGrity.git
   cd DroidGrity
   ```

2. **Install Python dependencies**:

    ```bash
    pip install -r requirements.txt
    ```

## How to use 🏃‍♂️

```
usage: python droidgrity.py [-h] [-v LOG_LEVEL] -a APK [-o OUTPUT] -ks KEYSTORE [-ksp KEYSTORE_PASS] [-ka KEY_ALIAS] [-kap KEY_PASS] [-sc SCHEMES [SCHEMES ...]] [-n ANDROID_NDK] [-ta ABIs [ABIs ...]] [-bt {Debug,Release}] [-i] [-nc]

options:
    -h, --help                              show this help message and exit
    -v, --log-level LOG_LEVEL               Logging level

APK:
    -a, --apk APK                           APK to protect
    -o, --output OUTPUT                     Protected APK

Signing:
    -ks, --keystore KEYSTORE                Keystore to use for signing
    -ksp, --keystore-pass KEYSTORE_PASS     Keystore password
    -ka, --key-alias KEY_ALIAS              Key alias
    -kap, --key-pass KEY_PASS               Key password
    -sc, --scheme SCHEMES [SCHEMES ...]     Signing scheme(s) to use

Dylib:
    -n, --android-ndk ANDROID_NDK           Path to Android NDK
    -ta, --target-abi ABIs [ABIs ...]       Android ABI(s) to target
    -bt, --build-type {Debug,Release}       Build type (mainly to enable/disable android logs)

Others:
    -i, --install                           Run ADB install
    -nc, --do-not-clean                     Do not clean temporary directories (for debugging purposes...)
```

# Examples

- **Basic usage**

```bash
python droidgrity.py -a APK_TO_PROTECT -n PATH_TO_ANDROID_NDK -ks KEYSTORE
```

- **With keystore authentication**

```bash
python droidgrity.py -a APK_TO_PROTECT -n PATH_TO_ANDROID_NDK -ks KEYSTORE -ksp KEYSTORE_PASSWORD -ka KEY_ALIAS -kap KEY_ALIAS_PASSWORD
```

- **Auto install to device**

```bash
python droidgrity.py -a APK_TO_PROTECT -ks KEYSTORE -n PATH_TO_ANDROID_NDK --install
```

## Known pitfalls

- Just patching the application with APKTool by removing the few lines used to call the integrity check JNI method
    - Can be made more difficult to do by using obfuscation to make it less obvious which lines should be removed
    - Even better use a packer so that patching would require patching an encrypted dex which will involve to fully reverse the packing mechanism

- Frida can be used to hook native methods and replace them so basically just have to replace the integrity check JNI method with a log
    - Implement additional anti-frida/hooking tools checks into the application
    - Obfuscating the native code to make it more difficult to know which native method to hook

## Possible further enhancements

- Allowing the user to define a handler in its code rather than crashing the application by default
- Use [o-llvm](https://github.com/obfuscator-llvm/obfuscator/wiki) to obfuscate the dylib

## License

This project is under BSD 3-Clause License. See [LICENSE](./LICENSE.md) for more details. 





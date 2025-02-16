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
- ☕ Java Development Kit (JDK)

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
usage: DroidGrity [-h] [-v LOG_LEVEL] -a APK [-o OUTPUT] -ks KEYSTORE [-ksp KEYSTORE_PASS] [-ka KEY_ALIAS] [-kap KEY_PASS] [-sc SCHEMES [SCHEMES ...]] [-n ANDROID_NDK] [-ta ABIs [ABIs ...]] [-bt {Debug,Release}] [-i] [-nc]

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

## License

This project is under BSD 3-Clause License. See [LICENSE](./LICENSE.md) for more details. 





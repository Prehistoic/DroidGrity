<!-- CSS --> 
blockquote {
  background-color: ##FAD5D3;
  border-left: 5px solid red;
}

<!-- Content -->

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

## License

This project is under BSD 3-Clause License. See [LICENSE](./LICENSE.md) for more details. 





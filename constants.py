import logging
import os

# Misc
LOG_LEVELS_MAPPING = {
    "DEBUG": logging.DEBUG,
    "INFO": logging.INFO,
    "WARNING": logging.WARNING,
    "ERROR": logging.ERROR
}

# Dynamic Library
ANDROID_ABIS = ["armeabi-v7a", "arm64-v8a", "x86", "x86_64"]
ANDROID_SIGNING_SCHEMES = ["v1", "v2", "v3", "v4"]
DYLIB_SRC_PATH = "cpp"
DYLIB_NAME = "droidgrity"
DYLIB_CPP_TEMPLATE = os.path.join(DYLIB_SRC_PATH, f"{DYLIB_NAME}.cpp.template")

# Smali
SMALI_SRC_PATH = "smali"
DYLIB_SMALI_TEMPLATE = os.path.join(SMALI_SRC_PATH, "DroidGrity.smali.template")

# Other paths
BUILD_DIR = "cpp/build"
BUILD_DYLIB_NAME = f"lib{DYLIB_NAME}.so"
TEMP_DIR = "temp"
INJECTED_APK_DIR = "injected"
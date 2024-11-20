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
DYLIB_SRC_PATH = "cpp"
DYLIB_CPP_TEMPLATE = os.path.join(DYLIB_SRC_PATH, "droidgrity.cpp.template")

# Paths
BUILD_DIR = "build"
BUILD_DYLIB_NAME = "libdroidgritty.so"
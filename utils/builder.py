import subprocess
import traceback
import shutil
import os
import logging

from constants import BUILD_DIR, BUILD_DYLIB_NAME

class CMakeBuilder:

    def __init__(self, target_android_sdk: str, target_abis: str, android_ndk_path: str = None):
        self.logger = logging.getLogger(__name__)

        self.target_abis = target_abis
        self.target_android_sdk = target_android_sdk
        self.android_ndk_path = android_ndk_path

        if not self.android_ndk_path:
            self.logger.info("No Android NDK path given, using ANDROID_NDK_ROOT from ENV...")
            try:
                self.android_ndk_path = os.environ["ANDROID_NDK_ROOT"]
            except KeyError:
                self.logger.error("ANDROID_NDK_ROOT missing from ENV")

    def build(self, src_dir: str):
        try:
            # Making sure we have all the inputs we need to run cmake
            if not self.android_ndk_path:
                self.logger.error("Missing Android NDK path, skipping build...")
                return []
            
            if not os.path.isdir(self.android_ndk_path):
                self.logger.error("Given Android NDK path is not a directory, skipping build...")
                return []
            
            if not self.target_android_sdk:
                self.logger.error("Missing target SDK to build, skipping build...")
                return []
            
            android_toolchain = f"{self.android_ndk_path}/build/cmake/android.toolchain.cmake"

            # Making sure cmake is in the PATH
            if not shutil.which("cmake"):
                self.logger.error("cmake is not in the PATH, skipping build...")
                return []

            # We clean and create the build directory architecture
            if os.path.exists(BUILD_DIR):
               self.logger.info(f"Cleaning {BUILD_DIR}...")
               shutil.rmtree(BUILD_DIR)
            
            self.logger.info(f"mkdir {BUILD_DIR}")
            os.mkdir(BUILD_DIR)

            # Finally we build the droidgrity dylib for the chosen abis
            built_dylibs = []
            for abi in self.target_abis:
                try:
                    self.logger.info(f"Building libdroidgrity.so for ABI {abi}...")
                    target_dir = os.path.join(BUILD_DIR, abi)
                    
                    configuration_cmd = f"cmake -S{src_dir} -B{target_dir} -DCMAKE_TOOLCHAIN_FILE={android_toolchain} -DANDROID_ABI={abi} -DANDROID_PLATFORM=android-{self.target_android_sdk} -DCMAKE_BUILD_TYPE=Release"
                    subprocess.run(configuration_cmd, shell=True, check=True, stderr=subprocess.PIPE, stdout=subprocess.DEVNULL, text=True)
                    
                    build_cmd = f"cmake --build {target_dir} --parallel"
                    subprocess.run(build_cmd, shell=True, check=True, stderr=subprocess.PIPE, stdout=subprocess.DEVNULL, text=True)

                    built_dylib = os.path.join(target_dir, BUILD_DYLIB_NAME)
                    self.logger.info(f"Built {built_dylib} successfully !")
                    built_dylibs.append(built_dylib)
                except subprocess.CalledProcessError:
                    self.logger.error(f"Failed to build for ABI {abi}\n{traceback.format_exc()}")

            return built_dylibs
        except Exception:
            self.logger.error(f"Error in build:\n{traceback.format_exc()}")
            return []
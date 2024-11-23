import shutil
import subprocess
import logging

class ApkInstaller:

    def __init__(self):
        self.logger = logging.getLogger(__name__)

    def install(self, apk_path: str):
        # First we verify that adb is in the PATH
        if not shutil.which("adb"):
            self.logger.error("adb is not in the PATH, skipping install...")
        
        else:
            try:
                install_cmd = f"adb install {apk_path}"
                subprocess.run(install_cmd, shell=True, check=True)
                self.logger.info(f"Installed {apk_path} to connected device...")
            except Exception:
                self.logger.error(f"Failed to install APK {apk_path} to connected device...")
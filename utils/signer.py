import logging
import traceback
import subprocess
import getpass
import shutil

class ApkSigner:

    def __init__(self, apk_path: str, keystore_path: str):
        self.logger = logging.getLogger(__name__)
        
        self.apk = apk_path
        self.keystore = keystore_path

    def sign(self, keystore_password: str, key_alias: str, key_password: str):
        # First we verify that both apksigner and zipalign are in the PATH
        if not shutil.which("apksigner"):
            self.logger.error("apksigner is not in the PATH, skipping signing...")
            return None
        
        if not shutil.which("zipalign"):
            self.logger.error("zipalign is not in the PATH, skipping signing...")
            return None

        try:
            self.logger.info("Running zipalign...")
            aligned_apk = self.apk.replace(".apk", "_aligned.apk")
            align_cmd = f"zipalign -p -f 4 {self.apk} {aligned_apk}"
            subprocess.run(align_cmd, shell=True, check=True)
            self.logger.info(f"APK aligned successfully => {aligned_apk}")
        
        except Exception:
            self.logger.error(f"Error in aligning:\n{traceback.format_exc()}")
            return None

        try:
            if not keystore_password or not key_alias or not key_password:
                print(" ")
                keystore_password = getpass.getpass("Keystore password: ") if not keystore_password else keystore_password
                key_alias = input("Key alias: ") if not key_alias else key_alias
                key_password = getpass.getpass("Key password: ") if not key_password else key_password
                print(" ")
            
            self.logger.info("Running apksigner...")
            signed_apk = aligned_apk.replace(".apk", "_signed.apk")
            sign_cmd = f"apksigner sign --ks {self.keystore} --ks-pass pass:{keystore_password} --ks-key-alias {key_alias} --key-pass pass:{key_password} --out {signed_apk} {aligned_apk}"
            subprocess.run(sign_cmd, shell=True, check=True)
            self.logger.info(f"Signed APK successfully => {signed_apk}")

            return signed_apk
        
        except Exception:
            self.logger.error(f"Error in signing...")
            return None
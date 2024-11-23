import logging
import traceback

from androguard.core.apk import APK
from androguard.util import set_log

class APKUtils:

    def __init__(self, path: str):
        set_log("ERROR")
        self.logger = logging.getLogger(__name__)
        
        self.path = path

    def get_package_name(self):
        try:
            apk = APK(self.path)
            return apk.get_package()
        except Exception:
            self.logger.error(f"Error reading APK:\n{traceback.format_exc()}")
            return None
    
    # Note : we are considering that if the APK has been signed using several schemes, the same certificate was used for all schemes
    def get_certificate_hash(self):
        try:
            apk = APK(self.path)
            certificates = apk.get_certificates()

            if certificates:
                cert_data = certificates[0]
                return cert_data.sha256_fingerprint
            else:
                self.logger.error("No certificates found in the APK")
                return None
        except Exception:
            self.logger.error(f"Error reading certificates:\n{traceback.format_exc()}")
            return None
        
    def get_min_sdk(self):
        try:
            apk = APK(self.path)
            return apk.get_min_sdk_version()
        except Exception:
            self.logger.error(f"Failed to retrieve minSdkVersion:\n{traceback.format_exc()}")
            return None
        
    def get_main_activity(self):
        try:
            apk = APK(self.path)
            return apk.get_main_activity()
        except Exception:
            self.logger.error(f"Failed to retrieve main activity:\n{traceback.format_exc()}")
            return None
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.serialization import pkcs12
from cryptography.hazmat.backends import default_backend
import traceback
import logging
import os

# Magic bytes for each supported keystore formats
KEYSTORE_FORMATS_MAGIC_BYTES = {
    "pkcs12": bytes([0x30, 0x82])
} 

def get_keystore_format(keystore_path: str):
    keystore_data = open(keystore_path, "rb").read()
    for format, magic_bytes in KEYSTORE_FORMATS_MAGIC_BYTES.items():
        if keystore_data.startswith(magic_bytes):
            return format
        
    return None

class Keystore:

    def __init__(self, path: str):
        self.logger = logging.getLogger(__name__)

        self.path = path
        self.format = None
        self.private_key = None
        self.certificate = None
        self.additional_certificates = []

    def load(self, password: str):
        self.logger.debug(f"Loading keystore {self.path}")

        try:
            if not os.path.exists(self.path):
                self.logger.error(f"Could not find any file at {self.path}")
                return False

            # First we need to identify which format is the given keystore
            self.format = get_keystore_format(self.path)
            if not self.format:
                self.logger.error(f"{self.path} does not match any supported keystore format ({', '.join(KEYSTORE_FORMATS_MAGIC_BYTES.keys())})")
                return False

            # Then we get the certificate from the keystore
            if self.format == "pkcs12":
                with open(self.path, "rb") as f:
                    private_key, certificate, additional_certificates = pkcs12.load_key_and_certificates(
                        f.read(), password.encode(), backend=default_backend()
                    )

                if not private_key and not certificate:
                    self.logger.error(f"Failed to load keystore {self.path}...")
                
                else:
                    self.logger.debug(f"Loaded keystore {self.path} with success")
                    self.private_key = private_key
                    self.certificate = certificate
                    self.additional_certificates = additional_certificates
            else:
                self.logger.error(f"The provided keystore format is not supported : {self.format}")
                return False
            
            self.logger.debug(f"Keystore {self.path} loaded successfully")
            return True
            
        except Exception:
            self.logger.error(f"Failed to load keystore {self.path}")
            self.logger.error(traceback.format_exc())
            return False
    
    def get_certificate_hash(self):
        if not self.certificate:
            self.logger.error("Can't compute certificate hash, no certificate was extracted. Have you tried running Keystore.load(...) first ?")
            return None

        try:
            cert_der = self.certificate.public_bytes(serialization.Encoding.DER)

            sha256_hash = hashes.Hash(hashes.SHA256(), backend=default_backend())
            sha256_hash.update(cert_der)

            certificate_hash_hex = sha256_hash.finalize()
            certificate_hash = certificate_hash_hex.hex()

            return certificate_hash

        except Exception:
            self.logger.error("Failed to compute certificate hash")
            self.logger.error(traceback.format_exc())
            return None

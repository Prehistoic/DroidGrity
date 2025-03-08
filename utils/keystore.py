from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.serialization import pkcs12
from cryptography.hazmat.backends import default_backend
import traceback
import logging

class Keystore:

    def __init__(self, path: str, password: str):
        self.logger = logging.getLogger(__name__)

        self.path = path

        self.logger.debug(f"Loading keystore {self.path}")

        with open(self.path, "rb") as f:
            private_key, certificate, additional_certificates = pkcs12.load_key_and_certificates(
                f.read(), password.encode(), backend=default_backend()
            )

        if not private_key and not certificate:
            self.logger.error(f"Failed to load keystore {self.path}...")
            self.private_key = None
            self.certificate = None
            self.additional_certificates = []
        
        else:
            self.logger.debug(f"Loaded keystore {self.path} with success")
            self.private_key = private_key
            self.certificate = certificate
            self.additional_certificates = additional_certificates
    
    def get_certificate_hash(self):
        if not self.certificate:
            self.logger.error("Can't compute certificate hash, no certificate was extracted on keystore load...")
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

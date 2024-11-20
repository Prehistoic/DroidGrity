import argparse
import logging
import sys
import os

from constants import ANDROID_ABIS, LOG_LEVELS_MAPPING, DYLIB_SRC_PATH, DYLIB_CPP_TEMPLATE
from utils.apk import APKUtils
from utils.filler import TemplateFiller
from utils.builder import CMakeBuilder

def droidgrity(args):
     # Setting up logging
    logger = logging.getLogger(__name__)
    logging.basicConfig(
        format="%(asctime)s > [%(levelname)s] %(message)s",
        datefmt="%d/%m/%Y %H:%M:%S",
        level=LOG_LEVELS_MAPPING.get(args.log_level),
    )

    # First we need to analyze the APK to retrieve its main package name + its signing certificate sha256 hash to fill tampercheck.cpp.template
    apk = APKUtils(args.apk)
    
    package_name = apk.get_package_name()
    if not package_name:
        logger.error("Failed to retrieve package name. Exiting...")
        sys.exit(-1)
    else:
        logger.info(f"APK - Package name = {package_name}")

    certificate_hash = apk.get_certificate_hash()
    if not certificate_hash:
        logger.error("Failed to retrieve certificate hash. Exiting...")
        sys.exit(-1)
    else:
        logger.info(f"APK - Certificate hash = {certificate_hash}")

    min_sdk = apk.get_min_sdk()
    if not min_sdk:
        logger.error("Failed to retrieve minSdkVersion. Exiting...")
        sys.exit(-1)
    else:
        logger.info(f"APK - minSdkVersion = {min_sdk}")
    
    # Then we fill droidgrity.cpp.template with the retrieved informations
    filler = TemplateFiller(DYLIB_CPP_TEMPLATE)
    data = {
        "appPackageName": package_name.replace(".", "_"),
        "knownCertHash": ", ".join(f"0x{part.lower()}" for part in certificate_hash.split())
    }
    filled_template = filler.fill(data)

    if not filled_template:
        sys.exit(-1)

    # Then we build libdroidgrity.so each one of the provided ABIs
    builder = CMakeBuilder(min_sdk, args.target_abi, args.android_ndk)
    built_dylibs = builder.build(DYLIB_SRC_PATH)
    
    os.remove(filled_template) # We delete the filled template since we do not need it anymore

    if not built_dylibs:
        sys.exit(-1)

    # Then we inject each libdroidgrity.so into the provided APK and we rebuild a new one

    # Finally we resign the newly built APK

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog="DroidGrity",
        description="Protecting Android apps from tampering with a custom dylib"
    )

    parser.add_argument("-v", "--log-level", dest="log_level", choices=LOG_LEVELS_MAPPING.keys(), metavar="LOG_LEVEL", help="Logging level", default="INFO", required=False)
    
    apk_args = parser.add_argument_group("APK")
    apk_args.add_argument("-a", "--apk", dest="apk", help="APK to protect", required=True)

    keystore_args = parser.add_argument_group("Keystore")
    keystore_args.add_argument("-ks", "--keystore", dest="keystore", help="Keystore to use for signing", required=True)
    keystore_args.add_argument("-ksp", "--keystore-pass", dest="keystore_pass", help="Keystore password", required=False) # Will be asked in CLI if not given
    keystore_args.add_argument("-ka", "--key-alias", dest="key_alias", help="Key alias", required=False) # Will be asked in CLI if not given
    keystore_args.add_argument("-kap", "--key-pass", dest="key_pass", help="Key password", required=False) # Will be asked in CLI if not given

    dylib_args = parser.add_argument_group("Dylib")
    dylib_args.add_argument("-n", "--android-ndk", dest="android_ndk", help="Path to Android NDK", required=False) # If not specified we'll use env variable ANDROID_NDK_ROOT
    dylib_args.add_argument("-ta", "--target-abi", dest="target_abi", nargs='+', choices=ANDROID_ABIS, metavar="ABIs", help="Android ABI(s) to target", default=ANDROID_ABIS, required=False)

    args = parser.parse_args()

    droidgrity(args)
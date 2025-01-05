import argparse
import logging
import shutil
import sys
import os

from constants import ANDROID_ABIS, LOG_LEVELS_MAPPING, DYLIB_SRC_PATH, DYLIB_CPP_TEMPLATE, DYLIB_SMALI_TEMPLATE, BUILD_DIR
from utils.apk import APKUtils
from utils.filler import TemplateFiller
from utils.builder import CMakeBuilder
from utils.injector import DylibInjector
from utils.signer import ApkSigner
from utils.installer import ApkInstaller

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

    main_activity = apk.get_main_activity().replace(".", "/")
    if not main_activity:
        logger.error("Failed to retrieve main activity. Exiting...")
        sys.exit(-1)
    else:
        logger.info(f"APK - Main Activity = {main_activity}")
    
    # Then we fill the different templates with the retrieved informations
    cpp_template_filler = TemplateFiller(DYLIB_CPP_TEMPLATE)
    data = {
        "appPackageName": package_name.replace(".", "_"),
        "knownCertHash": ", ".join(f"0x{part.lower()}" for part in certificate_hash.split())
    }
    filled_cpp_template = cpp_template_filler.fill(data)

    if not filled_cpp_template:
        logger.error(f"Failed to fill template {DYLIB_CPP_TEMPLATE}. Exiting...")
        sys.exit(-1)
    else:
        logger.info(f"Template filled with success => {filled_cpp_template}")

    smali_template_filler = TemplateFiller(DYLIB_SMALI_TEMPLATE)
    data = {
        "appPackageName": package_name.replace(".", "/"),
    }
    filled_smali_template = smali_template_filler.fill(data)

    if not filled_smali_template:
        logger.error(f"Failed to fill template {DYLIB_SMALI_TEMPLATE}. Exiting...")
        sys.exit(-1)
    else:
        logger.info(f"Template filled with success => {filled_smali_template}")

    # Then we build libdroidgrity.so each one of the provided ABIs
    builder = CMakeBuilder(min_sdk, args.target_abi, args.android_ndk)
    built_dylibs = builder.build(DYLIB_SRC_PATH)

    if len(built_dylibs) != len(args.target_abi):
        logger.error("Failed to build dylibs. Exiting...")
        sys.exit(-1)
    else:
        logger.info(f"Built dylibs => {', '.join(built_dylibs)}")

    # Then we inject each libdroidgrity.so into the provided APK and we rebuild a new one
    injector = DylibInjector(args.apk)
    injected_apk = injector.inject(main_activity, filled_smali_template)

    if not injected_apk:
        logger.error("Failed to inject dylib into APK. Exiting...")
        sys.exit(-1)
    else:
        logger.info(f"Injected dylib successfully. New APK => {injected_apk}")

    # Then we resign the newly built APK
    signer = ApkSigner(injected_apk, args.keystore)
    signed_apk = signer.sign(args.keystore_pass, args.key_alias, args.key_pass)

    if not signed_apk:
        logger.error("Failed to resign APK. Exiting...")
        sys.exit(-1)
    else:
        logger.info(f"Resigned APK successfully. Final APK => {signed_apk}")

    # Then we copy the newly build APK to its final output path
    try:
        output_apk_fullpath = args.output if args.output else os.path.join(os.path.dirname(args.apk), os.path.basename(args.apk).replace(".apk", "_protected.apk"))
        shutil.copy(signed_apk, output_apk_fullpath)
        logger.info(f"Copied signed APK {signed_apk} to {output_apk_fullpath}")
    except:
        logger.error(f"Failed to copy signed APK {signed_apk} to {output_apk_fullpath}. Exiting...")
        sys.exit(-1)

    # Finally we install the new APK if the option was enabled
    if args.install:
        installer = ApkInstaller()
        installer.install(output_apk_fullpath)

    logger.info("DONE :)")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog="DroidGrity",
        description="Protecting Android apps from tampering with a custom dylib"
    )

    parser.add_argument("-v", "--log-level", dest="log_level", choices=LOG_LEVELS_MAPPING.keys(), metavar="LOG_LEVEL", help="Logging level", default="INFO", required=False)
    
    apk_args = parser.add_argument_group("APK")
    apk_args.add_argument("-a", "--apk", dest="apk", help="APK to protect", required=True)
    apk_args.add_argument("-o", "--output", dest="output", help="Protected APK", required=False)

    keystore_args = parser.add_argument_group("Keystore")
    keystore_args.add_argument("-ks", "--keystore", dest="keystore", help="Keystore to use for signing", required=True)
    keystore_args.add_argument("-ksp", "--keystore-pass", dest="keystore_pass", help="Keystore password", required=False) # Will be asked in CLI if not given
    keystore_args.add_argument("-ka", "--key-alias", dest="key_alias", help="Key alias", required=False) # Will be asked in CLI if not given
    keystore_args.add_argument("-kap", "--key-pass", dest="key_pass", help="Key password", required=False) # Will be asked in CLI if not given

    dylib_args = parser.add_argument_group("Dylib")
    dylib_args.add_argument("-n", "--android-ndk", dest="android_ndk", help="Path to Android NDK", required=False) # If not specified we'll use env variable ANDROID_NDK_ROOT
    dylib_args.add_argument("-ta", "--target-abi", dest="target_abi", nargs='+', choices=ANDROID_ABIS, metavar="ABIs", help="Android ABI(s) to target", default=ANDROID_ABIS, required=False)

    other_args = parser.add_argument_group("Others")
    other_args.add_argument("-i", "--install", dest="install", action="store_true", help="Run ADB install", required=False)

    args = parser.parse_args()

    droidgrity(args)
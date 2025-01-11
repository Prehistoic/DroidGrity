import traceback
import logging
import shutil
import subprocess
import os

from constants import INJECTED_APK_DIR, BUILD_DIR, TEMP_DIR

class DylibInjector:

    def __init__(self, apk_path: str):
        self.logger = logging.getLogger(__name__)
        
        self.apk = apk_path

    def inject(self, target_activity: str, smali_file: str):
        self.logger.info(f"Attempting to inject DroidGrity.smali and loading dylibs in {target_activity}")

        # First we verify that apktool is in the PATH
        if not shutil.which("apktool"):
            self.logger.error("apktool is not in the PATH, skipping inject...")
            return None 

        try:
            if os.path.exists(TEMP_DIR):
                shutil.rmtree(TEMP_DIR)

            if os.path.exists(INJECTED_APK_DIR):
                shutil.rmtree(INJECTED_APK_DIR)

            # Step 1 : Decompile APK
            self.logger.info(f"Decompiling APK {self.apk} into {TEMP_DIR}...")
            decompile_cmd = f"apktool d {self.apk} -o {TEMP_DIR} -f"
            subprocess.run(decompile_cmd, shell=True, check=True, stderr=subprocess.PIPE, stdout=subprocess.DEVNULL, text=True)
            self.logger.info("Decompiled APK successfully")

            # Step 2 : Add the dylibs to the APK
            inserted_dylibs_paths = []
            for abi in [element for element in os.listdir(BUILD_DIR) if os.path.isdir(os.path.join(BUILD_DIR, element))]:
                lib_path_in_apk = os.path.join(TEMP_DIR, "lib", abi)
                os.makedirs(lib_path_in_apk, exist_ok=True)

                built_dylib_abi_dir = os.path.join(BUILD_DIR, abi)
                for lib_file in os.listdir(built_dylib_abi_dir):
                    if lib_file.endswith(".so"):
                        src_file = os.path.join(built_dylib_abi_dir, lib_file)
                        dst_file = os.path.join(lib_path_in_apk, lib_file)
                        shutil.copy(src_file, dst_file)
                        self.logger.info(f"Injected {src_file} into {dst_file}")

                        inserted_dylibs_paths.append(os.path.join("lib", abi, lib_file))

            # Step 2.5 : Add the new dylibs to the doNotCompress list in apktool.yml 
            # This is to avoid errors when installing APK which has extractNativeLibs = false in AndroidManifest.xml
            # We could also theoretically update the AndroidManifest.xml but I prefer not tampering with the original APK too much and since we got this alternative...
            with open(os.path.join(TEMP_DIR, "apktool.yml"), "r") as f:
                apktool_conf = f.readlines()

            for idx, line in enumerate(apktool_conf):
                if "doNotCompress:" in line:
                    apktool_conf.insert(idx + 1, "\n".join([f"- {dylib_path}" for dylib_path in inserted_dylibs_paths]) + "\n")
                    break

            with open(os.path.join(TEMP_DIR, "apktool.yml"), "w") as f:
                f.writelines(apktool_conf)
            
            # Step 3 : Insert DroidGrity.smali
            target_activity_package_name = "/".join(target_activity.split("/")[:-1])
            target_activity_name = target_activity.split("/")[-1]
            dst_file = os.path.join(TEMP_DIR, "smali", *(target_activity_package_name.split("/")), os.path.basename(smali_file))
            self.logger.info(f"Copying {smali_file} to {dst_file}")
            shutil.copy(smali_file, dst_file)
                
            # Step 4 : Modify smali files to load and use the dylibs
            target_activity_smali = os.path.join(TEMP_DIR, "smali", *(target_activity_package_name.split("/")), target_activity_name + ".smali")
            with open(target_activity_smali, "r") as f:
                smali_code = f.readlines()
            
            new_smali_code = []
            in_on_create = False
            idx = 0
            while idx < len(smali_code):
                line = smali_code[idx]

                if ".method protected onCreate(Landroid/os/Bundle;)V" in line:  # Hook into onCreate method
                    in_on_create = True
                elif in_on_create and ".locals" in line:
                    # We're updating the number of required registers if it is lower than the one we need in the code we will inject below
                    register_count = int(line.replace(".locals", "").strip())
                    line = f"    .locals 1" if register_count < 1 else line
                elif in_on_create and "return-void" in line:
                    # Basically we just add some code that will invoke the isApkTampered method from the DroidGrity.smali file we injected, before the end on the onCreate method
                    new_smali_code.append(f"    sget-object v0, L{target_activity_package_name}/DroidGrity;->INSTANCE:L{target_activity_package_name}/DroidGrity;" + "\n\n")
                    new_smali_code.append(f"    invoke-virtual {{v0}}, L{target_activity_package_name}/DroidGrity;->isApkTampered()Z" + "\n\n")
                    new_smali_code.append(f"    move-result v0" + "\n\n")
                    in_on_create = False
                
                # We add the original (or updated line) and go to the next line
                new_smali_code.append(line)
                idx += 1

            with open(target_activity_smali, "w") as f:
                f.writelines(new_smali_code)

            # Step 5 : Rebuild the APK
            self.logger.info(f"Rebuilding APK from {TEMP_DIR}")
            new_apk = os.path.join(INJECTED_APK_DIR, os.path.basename(self.apk).replace(".apk", "_injected.apk"))
            rebuild_cmd = f"apktool b {TEMP_DIR} -o {new_apk}"
            subprocess.run(rebuild_cmd, shell=True, check=True)
            self.logger.info(f"Rebuilded APK successfully => {new_apk}")

            return new_apk
        except Exception:
            self.logger.error(f"Failed to inject:\n{traceback.format_exc()}")
            return None

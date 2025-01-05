Use https://github.com/DimaKoz/stunning-signature/blob/master/app/src/main/c/path_helper.c to remove the fact that you need to get a path from Kotlin code to verify certs

Make it so the user can define a handler in its code rather than crashing the application
Maybe you could also just start the dylib code with JNI_OnLoad

Use o-llvm to obfuscate your library when building it ?

Compute hash of all certificates and integrate them all in the template ? Need to modify the check logic !

Also write the README, add a LICENCE...

Clean the code, better comments, smaller functions...
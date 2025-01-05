
Make it so the user can define a handler in its code rather than crashing the application
Maybe you could also just start the dylib code with JNI_OnLoad

Clean temporary directory and add an option to not clean them (for debugging purposes)

Use o-llvm to obfuscate your library when building it ?

Also write the README, add a LICENCE...

Clean the code, better comments, smaller functions...

Create a docker image ? would help to check what are the requirements to run the script without missing anything

AND RUN TESTS ! (different APKs, different arch...)
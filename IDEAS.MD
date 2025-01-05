You need to remove the apkpath that is passed as an argument to the JNI function !!!
Need to remove it in the smali template as well ;)
This will require to rewrite the smali code we inject into the onCreate from the main activity => attention to the locals this can mess up the method signature !

Add a banner

Make it so the user can define a handler in its code rather than crashing the application
Maybe you could also just start the dylib code with JNI_OnLoad

Think about what to do with the logs, remove them or make it possible to create a version without them for production !

Use o-llvm to obfuscate your library when building it ?

Also write the README, add a LICENCE...

Clean the code, better comments, smaller functions...

Create a docker image ? would help to check what are the requirements to run the script without missing anything

AND RUN TESTS ! (different APKs, different arch...)
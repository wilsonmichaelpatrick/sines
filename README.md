# sines

CAVEAT:  This project is definitely under construction.  Note that there is nothing substantial on the "master" branch; you are looking at development code.

Sine wave instrument that attempts to avoid pops due to discontinuities.

This project relies on the magnificent https://github.com/olilarkin/wdl-ol project to manage the building of the Audio Unit component.  See the likewise magnificent tutorial at http://www.martin-finke.de/blog/tags/making_audio_plugins.html for valuable information and examples.

Building the Audio Unit plugin (the only thing working so far):
```
git clone git@github.com:wilsonmichaelpatrick/sines.git
cd sines
git clone git@github.com:wilsonmichaelpatrick/wdl-ol.git
open au/sines/sines.xcodeproj
Go to sines->Frameworks And Libraries->Linked Frameworks->App & Plugins->lice.xcodeproj, search for ARCH under build settings for "lice_32&64_intel" and "lice_32_intel", and change the architecture to "64-bit Intel"
Switch to target AU -> My Mac (64-bit)
Cmd-B to build.
Output will go into the $HOME/Library/Audio/Plug-Ins/Components directory as sines.component
```

Notes:
```
Sometimes $HOME/Library/Caches/AudioUnitCache/com.apple.audiounits.cache needs to be removed for Logic to see the updated plugin.

Sometimes the whole machine needs to be rebooted for Logic to see the updated plugin.

auvaltool -v aumu Ipef Acme might not work if the steps described at https://stackoverflow.com/questions/39219678/ios-xcode-error-cannot-attach-to-process-due-to-system-integrity-protection have not been taken.

Using aumu Ipef Acme over and over again will confuse Logic as to the identity of different plugins.  That probably has something to do with the "don't forget to change PLUG_UID and MFR_UID in config.h" advice given at the end of the wdl-ol project generation.
```

# Distributed Training of Recurrent Neural Networks by FGM protocol

To resolve all project dependencies, in the root of the folder, in a terminal type
```
$ cd bash/
$ sudo bash setup-project-dependencies.bash
```
After that, in the same folder, to load cmake files and compile the custom library, type
```
$ sudo bash setup-project.bash
```
To compile a target (eg. GMNetEEG), in the root of the folder, type
```
$ cmake --build cmake-build-debug --target GMNetEEG
```
and to run this target, type
```
$ ./cmake-build-debug/bin/GMNetEEG absolute/path/to/simulation/gm_eeg_1.json
```


For more information about this work, check [here](tex/proposal/proposal.pdf) the thesis proposal.

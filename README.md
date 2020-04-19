# Distributed Training of Recurrent Neural Networks by FGM protocol #

To resolve all project dependencies, in the root of the folder, open a terminal and type:

```bash
cd bash/
chmod a+x *.bash
sudo bash setup-project-dependencies.bash
```

After that, in the same folder, to generate cmake files and compile the custom library, type:

```bash
bash setup-project.bash
```

To compile a target (eg. GMLearn), back to the root of the folder, type:

```bash
cmake --build cmake-build-debug --target GMLearn
```

and to run this target, type:

```bash
./cmake-build-debug/bin/GMLearn absolute/path/to/simulation/gm_eeg.json
```

For more information about this work, check [here](latex/proposal/proposal.pdf) the thesis proposal.

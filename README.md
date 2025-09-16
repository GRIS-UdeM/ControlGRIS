
<p align="left">
  <img width="150" src="https://github.com/user-attachments/assets/fbec48ec-1f0c-41f1-9a10-4ce993747f57">
  <img width="450" src="https://github.com/user-attachments/assets/4cd12fd2-1bab-4139-95a2-73bbfadde332">
</p>

# ControlGRIS

Spatialization plugin for [SpatGRIS](https://github.com/GRIS-UdeM/SpatGRIS). ControlGRIS is currently developed at the [Groupe de Recherche en Immersion Spatiale (GRIS)](https://gris.musique.umontreal.ca/) and the [Société des Arts Technologiques (SAT)](https://sat.qc.ca/en/).

## Building the ControlGRIS VST plugin on Debian (Ubuntu)

### Install dependencies

```
sudo apt-get install clang-15 git ladspa-sdk freeglut3-dev libasound2-dev libcurl4-openssl-dev libfreetype6-dev libjack-jackd2-dev libx11-dev libxcomposite-dev libxcursor-dev libxinerama-dev libxrandr-dev mesa-common-dev
```

### Clone ControlGRIS sources and submodules

```
git clone git@github.com:GRIS-UdeM/ControlGRIS.git
```
For the submodules:
```
cd ControlGRIS
git submodule update --init --recursive
```

### Build the Projucer

JUCE is included as a submodule. Go to `ControlGris/submodules/StructGRIS/submodules/JUCE/extras/Projucer/Builds/` and build the Projucer for your plateform.
Ensure that the Projucer global paths are set correctly. JUCE path is `ControlGris/submodules/StructGRIS/submodules/JUCE` and modules path is `ControlGris/submodules/StructGRIS/submodules/JUCE/modules`.

### Build the plugin

1. This step must be done each time the structure of the project changes (new files, new JUCE version, etc.).

```
<path/to/Projucer> --resave <path/to/ControlGRIS.jucer>
```

2. Make sure the directory `~/.vst` exists.

3. Go to the ControlGRIS Builds folder, compile the plugin and move a copy to the VST directory.

```
cd ControlGRIS/Builds/LinuxMakeFile
make CXX=clang++-15 CONFIG=Release && cp build/*.so ~/.vst/
```

4. Start Reaper and load the plugin!

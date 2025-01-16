# ControlGris

Spatialization plugin for [SpatGris](https://github.com/GRIS-UdeM/SpatGRIS)

## Building the ControlGris VST plugin on Debian (Ubuntu)

### Install dependencies

```
sudo apt-get install clang-15 git ladspa-sdk freeglut3-dev libasound2-dev libcurl4-openssl-dev libfreetype6-dev libjack-jackd2-dev libx11-dev libxcomposite-dev libxcursor-dev libxinerama-dev libxrandr-dev mesa-common-dev
```

### Download Juce

Download and extract [Juce 8.0.6](https://github.com/juce-framework/JUCE/releases/tag/8.0.6)

### Download the Steinberg VST SDK

The VST3 headers are included in JUCE, but the VST2 are not. The VST2 SDK is deprecated and not distributed anymore by Steinberg. You will need a copy of it if you are to build the VST2 plugin. Extract it somewhere and add it the Projucer global paths.

### Clone ControlGris sources

```
git clone git@github.com:GRIS-UdeM/ControlGris.git
```

### Build the plugin

1. This step must be done each time the structure of the project changes (new files, new JUCE version, etc.).

```bash
<path/to/Projucer> --resave <path/to/ControlGris.jucer>
```

2. Make sure the directory `~/.vst` exists.

3. Go to the ControlGris Builds folder, compile the plugin and move a copy to the VST directory.

```
cd ControlGris/Builds/LinuxMakeFile
make CXX=clang++-15 CONFIG=Release && cp build/*.so ~/.vst/
```

4. Start Reaper and load the plugin!

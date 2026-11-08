# VidCom

VidCom (short for Video Compression) is a simple utility for archiving videos. It offers both GUI and CLI interfaces and utilises `ffmpeg` for the video encoding. 

For screenshots and basic information, visit the [project website](https://seja-arctic-fox.github.io). 

VidCom features two modes; **Archive mode** and **Compress mode**:

- **Archive mode** compresses a video as much as possible without losing target image quality. 

- **Compress mode** compresses a video to a specified target size. 

There are also other options for quick setup, such as reducing resolution and frame rate, choosing an output directory and a trimming function. More advanced users can set the codec used for encoding and tweak some parameters of chosen codec to trade time for better compression and vice versa. 

You can process as many videos as you want by adding them to a queue (GUI) or specifying their paths in the command (CLI). 

Whether you want your memories to take up less space or just share a funny video clip with your friends, this app makes it easy for you. 

### Features
- Archiving videos
- Compressing videos to target sizes
- Simple and clean GTK4 + Adwaita user interface
	* Adjustable video queue
	* Simple way of setting the basics and advanced options for single or multiple videos
	* Start encoding with a single button
- CLI for automation
- Uses `ffmpeg` for encoding
- Avaiable codecs are: 
    * AV1 `libsvtav1`
    * HEVC `libx265`
    * VP9 `libvpx-vp9`
    * AVC `libx264`
- Written in C++

### CLI
When you run the command `vidcom`, it start the GUI by default. To use the CLI interface, you have to specify the `-cli` argument and at least one input file. To see the available commands and some example usage of the CLI, execute the following: 

```bash
vidcom -h
```

On Windows, there are two separate executables: `vidcom.exe` for GUI and `vidcom-cli.exe` for CLI. The `-cli` argument is thus omitted. 

```powershell
.\vidcom-cli.exe -h
```

### Inspiration
This project was inspired mainly by these other projects: 

- [Constrict](https://github.com/Wartybix/Constrict)
- [Handbrake](https://handbrake.fr/)

## Installation
This project is mainly developed for Linux. It can be built on Linux and Windows OSses. See sections below for guidance. 

### Linux
#### AUR
```bash
yay -S vidcom
```

#### Flatpak
You can get VidCom on [Flathub](https://flathub.org/en/apps/io.github.seja_arctic_fox.vidcom)

Beta versions are also avaiable during development. You can test them by adding the `flathub-beta` repository and installing the app from there. 

#### Building for Linux
1. Install dependencies: `meson`, `ffmpeg`, `gtk4`, `gtkmm-4.0`, `libadwaita` and `jsoncpp`. For example on Arch Linux: 

```bash
pacman -S meson ffmpeg gtk4 libadwaita jsoncpp
```

2. Clone this repository: 

```bash
git clone https://github.com/seja-arctic-fox/vidcom.git
cd vidcom
```

3. Setup the build directory:

```bash
meson setup build
```

4. Compile and install: 

```bash
cd build/

# COMPILING WITHOUT INSTALLATION
meson devenv
meson compile

# COMPILE AND INSTALL ON YOUR SYSTEM
meson install
```

---

If you want to compile and install as a flatpak package, execute these commands in the root directory after cloning:
```bash
flatpak remote-add --if-not-exists --user flathub https://dl.flathub.org/repo/flathub.flatpakrepo
# Dont forget to install flatpak-builder
flatpak-builder --force-clean --user --install-deps-from=flathub --repo=repo --install flatpak_builddir ./data/*.yml
```

---

If you want to compile and install as an Pacman package, execute this command in the root directory after cloning:
```bash
makepkg -si
```

### Windows
Windows builds are available since the release of version `0.84` in the releases as a portable ZIP archive. 

Before using the program, make sure you have `ffmpeg` installed in `%PATH%` or the executable is included next to `vidcom.exe`. The easiest way of installing `ffmpeg` is using `winget`:

```bat
winget install Gyan.ffmpeg
```

#### Building for Windows
1. Install MSYS2: \
```bat
winget install MSYS2.MSYS2
```
2. Enter the UCRT64 environment
3. Install packages \
```bash
pacman -Syu
pacman -S $MINGW_PACKAGE_PREFIX-{glib2,gtk4,gtkmm-4.0,libadwaita,glibmm-2.68,jsoncpp,git,meson,ninja}
```
4. Clone the repository
5. Create an install directory, compile and install VidCom into it: \
```bash
# make sure you are in the repository's root directory
mkdir install
meson setup build --prefix=$(pwd)/install
meson devenv
meson compile
meson install # after this, you can exit out of dev environment
```
6. Copy all dynamically linked libraries outside `C:\Windows` next to the executable: \
```bash
ldd ./install/bin/vidcom.exe | awk '/ucrt64/ {print $3}' | xargs -I{} cp {} ./install/bin/
find /ucrt64/bin -name "gspawn*" -exec cp {} ./install/bin/ \;
```
7. Find all included icons and copy them to `./install/share/icons` \
```bash
# Search all source files and extract included icons
grep -rhoE '"[a-z0-9.-]+-symbolic"' src | tr -d '"' | sort -u > icons.txt
# Exclude already included icons
ls data/icons | sed 's/\.svg$//' | sort > own.txt
comm -23 icons.txt own.txt > needed.txt
# Copy them
for l in $(cat ./build/needed.txt); do find /ucrt64/share/icons/Adwaita -name "$l.svg" -exec cp {} ./install/share/icons/hicolor/scalable/actions/ \;; done
```

## Conclusion

I will continue to maintain this project/add new features as new codecs become available. 

There's still a lot of improvements/changes to be made. The app will probably be restructured as it moves towards 1.0 release. 

If you have a suggestion or you found a bug, feel free to report it in the [Issues](https://github.com/seja-arctic-fox/vidcom/issues) page. Check the list below or the issues to see if it has been reported before. 

I will update the lists below from time to time;

### Planned features / suggestions
- [x] At least try to make a Windows build
- [ ] Implement translations (or ability to translate the app)
- [ ] Copy settings from one video to another with right click/shortcut/button
- [ ] Keyboard shortcuts
- [x] Different behavior for cases when the output videos already exist - show conflicts
- [ ] Better TAB behavior + accelerators
- [ ] Implement functional 2-pass encoding for all codecs
- [ ] Think about a settings page which could be easier to use and understand

### Known Issues
- [ ] Encoding can fail when using specific parameters for certain codecs, potentially missing UI restrictions

This project follows [GNOME's Code of Conduct](https://conduct.gnome.org/).

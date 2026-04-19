# VidCom

VidCom (short for Video Compression) is a simple utility for compressing video. It offers both GUI and CLI interface and utilises `ffmpeg` for the video encoding. 

For screenshots and basic information, visit the [project website](https://seja-arctic-fox.github.io). 

VidCom features two modes; **Archive mode** and **Compress mode**:

- **Archive mode** compresses a video as much as possible without losing target image quality. 

- **Compress mode** compresses a video to a specified target size. 

There are also other options for quick setup, such as reducing resolution and frame rate, choosing an output directory and a trimming function. More advanced users (and anyone else) can set the codec used for encoding and tweak some parameters of chosen codec to trade time for better compression and vice versa. 

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
    * VP9 ` libvpx-vp9`
- Written in C++

### CLI
When you run the command `vidcom`, it start the GUI by default. To use the CLI interface, you have to specify at least one input file. To see the avaiable commands and some example usage of the CLI, execute the following: 

```bash
vidcom -h
```

### Inspiration
This project was inspired mainly by these other projects: 

- [Constrict](https://github.com/Wartybix/Constrict)
- [Handbrake](https://handbrake.fr/)

## Installation
This project is mainly developed for Linux. I might do a Windows build in the future. 

### Linux
#### AUR
```bash
yay -S vidcom
```

#### Flatpak
You can get VidCom on [Flathub](https://flathub.org/en/apps/io.github.seja_arctic_fox.vidcom)

#### Building
1. Install dependencies: `meson`, `ffmpeg`, `gtk4`, `libadwaita` and `jsoncpp`. Alternatively change `gtk4` for `gtkmm-4.0`. For example on Arch Linux: 

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

meson compile
# OR
meson install
```

### Windows
#### WSL
Tested on WSL with Ubuntu and Arch Linux, worked great OOTB, but I need to test it more. 

#### Other
Maybe I could do it with `msys2`, but it is not top priority. 
I will make a Windows build only if it is not unecessary hard to do so. 

## Conclusion

I will continue to maintain this project/add new features as new codecs become avaiable. 

There's still a lot of improvements/changes to be made. The app will probably be restructured as it moves towards 1.0 release. The ultimate goal is to join the **GNOME Circle**. 

If you have a suggestion or you found a bug, feel free to report it in the [Issues](https://github.com/seja-arctic-fox/vidcom/issues) page. Check the list below or the issues to see if it has been reported before. 

I will update the lists below from time to time;

### Planned features / suggestions
- [ ] At least try to make a Windows build
- [ ] Implement translations (or ability to translate the app)
- [ ] About window/section
- [ ] Add an ability to change default configuration/settings. save the configs in a file
- [ ] Change the current paned window split into the typical Adwaita side panel for queue and the main page
- [ ] "No video selected" page for the settings page
- [ ] Copy subtitles automatically when they are present in the original video
- [ ] Be able to select multiple videos manually, not be limited to just selecting single or all the videos (when holding Shift or other key)
- [ ] Make the runner (in the header bar) slimmer and simpler
- [ ] Show state of loading/encoding in the settings page space
- [ ] Highlight the currently encoding video in the queue
- [ ] Copy settings from one video to another with right click/shortcut/button
- [ ] Keyboard shortcuts

### Known Issues
- [ ] Cut feature - change spinbuttons for something more manageable, specifically for time setting + make it less rigid
- [ ] The button in the results page is not visible when more results are displayed (it's on the bottom), which might be confusing

This project follows [GNOME's Code of Conduct](https://conduct.gnome.org/).

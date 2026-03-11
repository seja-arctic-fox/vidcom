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

Of course this app is not perfect and definitely can be improved. Still, I hope you will find it useful, at least a little! :D 

I will continue to maintain this project/add new features as new codecs become avaiable, but I might be slow to implement them. 

If you have a suggestion or you found a bug, feel free to report it in the [Issues](https://github.com/seja-arctic-fox/vidcom/issues) page. Check the list below or the issues to see if it has been reported before. 

I will update the lists below from time to time;

### Planned features / suggestions
- [ ] Bring the app to
	- [X] Flatpak
	- [X] AUR
	
	~~- Make a AppImage~~
	- [ ] winget?
- [ ] At least try to make a Windows build
- [ ] Implement translations (or ability to translate the app)
- [ ] About window/section
- [ ] Notify when the encoding finishes? (maybe don't focus the window then)
- [ ] Add an ability to change default configuration/settings. save the configs in a file
- [ ] Make the queue hide when shrinking the Window + adding a button to hide the shrink/expand the queue

### Known Issues
- [ ] Box for widgets in the "Cut Feature" section is rigid, the plan is to make a flex box that behaves normally. Currently, it blocks the window from resizing
- [ ] The app doesn't show any status change when importing videos. When importing more videos at the same time, the app will appear non-active. It should show a "Loading" status at the top at least
- [ ] CLI doesn't create a newline after finishing encoding (regression)
- [ ] The description of present when HEVC codec is selected is wrong, it should be the other way around
- [ ] The batch editing does not work for video codec parameters. Either block the usage of this section or (preferably) implement a fix. Maybe block it if all the videos are not set to the same codec
- [x] Problem with the "Open video" pill button on the "results" page; sometimes it does not open a video at all. The problem may be trivial (wrong path supplied) and should be fixed easily. 
- [ ] The app crashes when it attempts to write files/folders to a restricted location. It should notify the user  

This project follows [GNOME's Code of Conduct](https://conduct.gnome.org/).

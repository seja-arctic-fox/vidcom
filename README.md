# VidCom

VidCom (short for Video Compression) is a simple utility for compressing video. It offers both GUI and CLI interface and utilises `ffmpeg` for the video encoding. 

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

### Inspiration
This project was inspired mainly by these other projects: 

- [Constrict](https://github.com/Wartybix/Constrict)
- [Handbrake](https://handbrake.fr/)

## Installation
This project is mainly developed for Linux. I might do a Windows build in the future. 

### Linux
#### AUR
TODO

```bash
yay -S vidcom
```

#### Flatpak
TODO

#### Building
1. Install dependencies: `meson`, `ffmpeg`, `gtk4`, `libadwaita` and `jsoncpp`. For example on Arch Linux: 

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
Maybe someday, when I figure out how to do it and prove that it will work at least somewhat decently. 

## Conclusion

Of course this app is not perfect and definitely can be improved. Still, I hope you will find it useful, at least a little! :D 

Todo (I will update this with some future ideas):

- Add two-pass encoding to GUI (and figure out how to do 2-pass encoding with `libsvtav1` properly)
- Improving the GUI to be more like a typical application from the GNOME Circle
- ...

If you have a suggestion or you found a bug, feel free to report it in the [Issues](https://github.com/seja-arctic-fox/vidcom/issues) page. 

I will continue to maintain this project/add new features as new codecs become avaiable, but I might be slow to implement them. 

This project follows [GNOME's Code of Conduct](https://conduct.gnome.org/).
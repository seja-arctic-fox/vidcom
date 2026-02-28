project(
  'vidcom-gui',
  'cpp',
  version : '0.8',
  meson_version : '>= 1.3.0',
  default_options : ['warning_level=3', 'cpp_std=c++20'],
)

gtkmm_dep = dependency('gtkmm-4.0', version: '>= 4.6.0')
adwaita_dep = dependency('libadwaita-1')
jsoncpp = dependency('jsoncpp')
threads = dependency('threads')

dependencies = [
  gtkmm_dep, 
  adwaita_dep,
  jsoncpp,
  threads
]

gui = [
  './src/gui/gui.cpp',
  './src/gui/queue_element.cpp',
  './src/gui/queue_frame.cpp',
  './src/gui/runner_title_bar.cpp',
  './src/gui/settings_page.cpp',
  './src/gui/av1_page.cpp',
  './src/gui/hevc_page.cpp',
  './src/gui/vp9_page.cpp',
  './src/gui/results_dialog.cpp'
]

video = [
  './src/video/video.cpp',
  './src/video/encoding.cpp',
  './src/video/setters.cpp',
  './src/video/getters.cpp'
]

cli = [
  './src/cli/print.cpp',
  './src/cli/parsing.cpp',
  './src/cli/setters.cpp'
]

exe = executable(
  'vidcom',
  'vidcom_gui.cpp',
  gui, video, cli,
  install : true,
  dependencies : dependencies,
)

test('basic', exe)


datadir = get_option('datadir')
metainfodir = datadir / 'metainfo'
desktopdir = datadir / 'applications'
icondir = datadir / 'icons' / 'hicolor' / 'scalable' / 'apps'

install_data (
  'data/io.github.seja_arctic_fox.vidcom.desktop',
  install_dir: desktopdir
)

install_data(
  'data/io.github.seja_arctic_fox.vidcom.metainfo.xml',
  install_dir: metainfodir
)

install_data(
  'data/io.github.seja_arctic_fox.vidcom.svg',
  install_dir: icondir
)
datadir = get_option('datadir')
settingsdir = datadir / 'glib-2.0/schemas'

install_data(
  'io.github.seja_arctic_fox.vidcom.gschema.xml',
  install_dir: settingsdir
)

gnome = import('gnome')
gnome.post_install(
  glib_compile_schemas: true,
)
gnome.compile_schemas()

test('validate-schema',
  find_program('glib-compile-schemas'),
  args: ['--strict', '--dry-run', meson.current_source_dir()],
)

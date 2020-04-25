project('tutorial', 'objcpp')
add_global_arguments('-std=c++17', language : 'objcpp')

metal_path = run_command('xcrun', '-sdk', 'macosx', '--find', 'metal').stdout().strip()
metallib_path = run_command('xcrun', '-sdk', 'macosx', '--find', 'metallib').stdout().strip()

metal_comp = find_program(metal_path)
metallib_comp = find_program(metallib_path)

air_gen = generator(metal_comp, output : '@BASENAME@.air', arguments : ['-c', '@INPUT@', '-o', '@OUTPUT@'])
air_src = air_gen.process('Shader.metal')
default_metallib = custom_target(
    'default.metallib',
    output : 'default.metallib',
    input : air_src,
    command : [metallib_comp, '@INPUT@', '-o', '@OUTPUT@'],
    install : true,
    install_dir : 'Contents/Resources'
)

dep_main = dependency('appleframeworks', modules : ['Foundation', 'Cocoa', 'Metal', 'MetalKit'])

executable('example', 'main.mm', install : true, dependencies: dep_main)
install_data('example.icns', install_dir : 'Contents/Resources')
install_data('Info.plist', install_dir : 'Contents')
install_data('blocks.png', install_dir : 'Contents/Resources')

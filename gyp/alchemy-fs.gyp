{
  'includes': [ 'common.gypi' ],
  'targets': [
    {
      'target_name': 'alchemy-fs',
      'product_name': 'alchemy-fs',
      'type': 'executable',
      'sources': [
        '../src/alchemy-fs.c',
      ],
      'include_dirs': [
        '/usr/local/cuda/include',
      ],
      'cflags' : [
        '<!@(pkg-config fuse --cflags)'
      ],
      'link_settings': {
        'libraries' : [
          '-lcudart',
          '<!@(pkg-config fuse --libs)'
        ],
        'ldflags' : [
          '-L/usr/local/cuda/lib64'
        ]
      },
    },
    {
      'target_name': 'alchemy-fs-test',
      'product_name': 'alchemy-fs-test',
      'type': 'executable',
      'sources': [
        '../test/alchemy-fs-test.cc',
      ],
    },
  ],
}

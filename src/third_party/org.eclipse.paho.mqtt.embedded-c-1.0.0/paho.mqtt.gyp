{
  'targets': [
  {
    'target_name': 'MQTTPacket',
    'type': 'static_library',
    'direct_dependent_settings': {
      'include_dirs': [
        'MQTTPacket/src',
      ],
    },
    'includes': [
      'paho.mqtt.srcs.gypi',
    ],
    'sources': [
      '<@(MQTTPacketSources)'
    ],
  }
  ]
}

{
  'targets': [
  {
    'target_name': 'mqttclient',
    'type': 'static_library',
    'direct_dependent_settings': {
      'include_dirs': [
        'include',
      ],
    },
    'includes': [
      'mqttclient.srcs.gypi',
    ],
    'include_dirs':[
      'include',
    ],
    'sources': [
      '<@(MQTTClientSources)',
    ],
    'dependencies': [
      '../org.eclipse.paho.mqtt.embedded-c-1.0.0/paho.mqtt.gyp:MQTTPacket',
      '../../base/base.gyp:base',
    ]
  }
  ]
}

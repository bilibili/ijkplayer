Pod::Spec.new do |s|
  s.name               = "ijkplayer"
  s.version            = "1.2.0"
  s.summary            = "Android/iOS video player based on FFmpeg n2.8, with MediaCodec, VideoToolbox support."
  s.homepage           = "https://github.com/ArenaCloud/ijkplayer"
  s.author             = "ArenaCloud"
  s.license            = { :type => "LGPL", :file => "COPYING.LGPLv3" }
  s.platform           = :ios, "7.0"
  s.source             = { :git => 'https://github.com/ArenaCloud/ijkplayer.git', :tag => 'v' << s.version.to_s }
  s.source_files       = "SDK/iOS/include/*.{h,m,c}"
  s.requires_arc       = true

  s.frameworks         = [ 'VideoToolbox', 'AudioToolbox', 'AVFoundation', 'CFNetwork', 'CoreMedia',
                            'CoreVideo', 'OpenGLES', 'Foundation', 'CoreGraphics' ]
  s.libraries          = [ 'c++', 'z' ]

  s.subspec 'include' do |ss|
    ss.source_files        = "SDK/iOS/include/*.h"
    ss.vendored_library   = "SDK/iOS/lib/libIJKMediaPlayer.a"
  end
end
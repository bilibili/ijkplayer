Pod::Spec.new do |s|

  s.name         = "ijkplayer"
  s.version      = "0.3.2-rc.3"
  s.summary      = "Video player based on FFmpeg n2.7"
  s.homepage     = "https://github.com/Bilibili/ijkplayer"

  s.license      = { :type => 'LGPLv2.1+', :file => 'COPYING.LGPLv2.1.txt' }
  s.author       = { "Zhang Rui" => "bbcallen@gmail.com" }

  s.platform     = :ios, '6.0'

  s.source       = { :git => "https://github.com/Bilibili/ijkplayer.git", :tag => "k#{s.version}" }

  s.frameworks     = 'AudioToolbox', 'AVFoundation', 'CoreAudio', 'CoreGraphics', 'CoreMedia', 'CoreVideo', 'MediaPlayer', 'OpenGLES', 'UIKit', 'QuartzCore'
  s.weak_framework = 'VideoToolbox'


  s.subspec 'IJKMediaPlayer' do |ss|
    ss.dependency 'ijkplayer/ijkplayer-ios'
    ss.dependency 'ijkplayer/ijkplayer-ios-mrc'

    ss.source_files         = 'ios/IJKMediaPlayer/IJKMediaPlayer/*.{h,m}'
    ss.public_header_files  = [
      'ios/IJKMediaPlayer/IJKMediaPlayer/IJKMediaPlayer.h',
      'ios/IJKMediaPlayer/IJKMediaPlayer/IJKMediaModule.h',
      'ios/IJKMediaPlayer/IJKMediaPlayer/IJKMediaPlayback.h',
      'ios/IJKMediaPlayer/IJKMediaPlayer/IJKMPMoviePlayerController.h',
      'ios/IJKMediaPlayer/IJKMediaPlayer/IJKAVMoviePlayerController.h',
      'ios/IJKMediaPlayer/IJKMediaPlayer/IJKFFMoviePlayerController.h',
      'ios/IJKMediaPlayer/IJKMediaPlayer/IJKFFOptions.h']
    ss.header_mappings_dir  = 'ios/IJKMediaPlayer'

    ss.exclude_files        = 'ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia/**/*'
end


  s.subspec 'ijkplayer-ios' do |ss|
    ss.dependency 'ijkplayer/ijkplayer'
    ss.dependency 'ijkplayer/ijksdl-ios'
    ss.dependency 'ijkplayer/ijksdl-ios-mrc'

    ss.source_files         = 'ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia/ijkplayer/**/*.{c,h,m}'
    ss.private_header_files = 'ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia/ijkplayer/**/*.h'
    ss.header_mappings_dir  = 'ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia'

    ss.exclude_files        = 'ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia/ijkplayer/ios/ijkplayer_ios.m'
end

  s.subspec 'ijkplayer-ios-mrc' do |ss|
    ss.dependency 'ijkplayer/ijkplayer'
    ss.dependency 'ijkplayer/ijksdl-ios'
    ss.dependency 'ijkplayer/ijksdl-ios-mrc'

    ss.requires_arc         = false;
    ss.source_files         = [
    'ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia/ijkplayer/ios/ijkplayer_ios.m',
    'ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia/ijkplayer/**/*.h']
    ss.private_header_files = 'ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia/ijkplayer/**/*.h'
    ss.header_mappings_dir  = 'ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia'
end

  s.subspec 'ijksdl-ios' do |ss|
    ss.dependency 'ijkplayer/ijksdl'

    ss.source_files         = 'ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia/ijksdl/**/*.{c,h,m}'
    ss.private_header_files = 'ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia/ijksdl/**/*.h'
    ss.header_mappings_dir  = 'ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia'

    ss.exclude_files        = ['ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia/ijksdl/ios/ijksdl_aout_ios_audiounit.m', 'ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia/ijksdl/ios/ijksdl_vout_ios_gles2.m']
end

  s.subspec 'ijksdl-ios-mrc' do |ss|
    ss.dependency 'ijkplayer/ijksdl'

    ss.requires_arc         = false;
    ss.source_files         = ['ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia/ijksdl/ios/ijksdl_aout_ios_audiounit.m', 'ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia/ijksdl/ios/ijksdl_vout_ios_gles2.m',
    'ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia/ijksdl/**/*.h']
    ss.private_header_files = 'ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia/ijksdl/**/*.h'
    ss.header_mappings_dir  = 'ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia'
end


  s.subspec 'ijkplayer' do |ss|
    ss.dependency 'ijkplayer/ijksdl'

    ss.source_files         = 'ijkmedia/ijkplayer/**/*.{c,h,m}'
    ss.private_header_files = 'ijkmedia/ijkplayer/**/*.h'
    ss.exclude_files        = 'ijkmedia/ijkplayer/android/**/*'
    ss.header_mappings_dir  = 'ijkmedia'
end

  s.subspec 'ijksdl' do |ss|
    ss.dependency 'FFmpeg4ijkplayer-ios-bin', '0.3.1-rc.7'

    ss.source_files         = 'ijkmedia/ijksdl/**/*.{c,h}'
    ss.private_header_files = 'ijkmedia/ijksdl/**/*.h'
    ss.exclude_files        = 'ijkmedia/ijksdl/android/**/*'
    ss.header_mappings_dir  = 'ijkmedia'
end

end

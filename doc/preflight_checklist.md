ChangeLog
-----------------------
* Update NEWS.md
* Update README.md
* Commit and push

FFmpeg
-----------------------
* Build and test iOS and Android demo locally
* Modify ffmpeg version in init-ios.sh and init-android.sh
* Modify ffmpeg version in `IJKFFMoviePlayerController` (by running `sh init-ios.sh`)
* Commit and push

ijkplayer
-----------------------
* Update version.sh
* Create a tag (subtitle)
* Commit and push (TAG ONLY)

Travis-ci
-----------------------
* Modify ijk version in `.travis.yaml` in iOS and Android ci repo.
* Ensure compile task has been started on travis-ci.
* Ensure Andoird release has been released in bintray.

Take off
-----------------------
* Push master to github

# MediaCodec H.264 output-size host regressions

```sh
bash run_ffamc_output_size_tests.sh /tmp/ijk-output-size
SANITIZE=1 bash run_ffamc_output_size_tests.sh /tmp/ijk-output-size-sanitized
CFLAGS='--coverage' bash run_ffamc_output_size_tests.sh /tmp/ijk-output-size-coverage
```

These tests compile the production output-size adapter with minimal pipeline
dependency types. They require only a C99 host compiler, without FFmpeg, JNI,
network access, media URLs or a physical decoder.

The regression reproduces a 1920x1080 ad followed by 1280x536 content while the
MediaCodec allocation stays 1920x1080. It verifies the next frames use the
visible crop, including ad/content/ad, repeated notifications, offset and
single-axis crop changes, rotation, absent/partial/invalid crop, integer bounds,
and preservation of the existing HEVC, MPEG4 and TI H.264 behavior.

Both output dequeue paths call the same adapter. `amc_fill_frame` copies the
updated dimensions into each frame, and the existing `queue_picture` path
publishes size changes. No manual size callback, codec reconfiguration,
buffer reallocation or packet/extradata rewrite is introduced here.

These are host tests of the production adapter, not Android hardware acceptance.
On-device validation should retain real MediaCodec decoding, cross a crop-only
change, and check the reported video size, rendered aspect ratio and advancing
playback. Square-pixel 1280x536 video fits a 1920x1080 viewport as 1920x804 with
138-pixel top and bottom margins. Test each output dequeue mode and vendor
separately; TI remains excluded from this change.

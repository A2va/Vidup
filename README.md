# Vidup

The upscaler works now but it is quite slow (104 seconds to upscale 4 seconds of 720 *2 video). I will try to improve by using concurential programming.

This is a video upscaler (like [video2x](https://github.com/k4yt3x/video2x),[dandere2x](https://github.com/akai-katto/dandere2x))
but the main difference with this one is that it does not extract the images from the video to put them in a temporal folders.

Most codecs and containers will be supported because the project uses the ffmpeg library as encoder/decoder. For the image it use the superres module of OpenCV.

*Thanks to [Olive develloper](https://github.com/olive-editor/olive) who helped me to code the encoder and decoder class.*
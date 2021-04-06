# Vidup

**Note: for the moment the project is too new and does absolutely nothing.**

This is a video upscaler (like [video2x](https://github.com/k4yt3x/video2x),[dandere2x](https://github.com/akai-katto/dandere2x))
but the main difference with this one is that it does not extract the images from the video to put them in a temporal folders.

Most codecs and containers will be supported because the project uses the ffmpeg library as encoder/decoder. For the image it use the superres module of OpenCV.

*Thanks to [Olive project](https://github.com/olive-editor/olive) who helped me to code the encoder and decoder class.*
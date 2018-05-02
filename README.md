# Image Mosaic Wall
This application allows you to create an image based on a bunch of other images. It looks like a mosaic effect. But on detail you can check the following video.

[![Video](https://github.com/scheckmedia/ImageMosaicWall/blob/master/docs/placeholer.jpg?raw=true)](https://scheck-media.de/tmp/imw.mp4)


## TODO's
- [ ] Drag & Drop for a single image
- [x] a better history algorithm (something like a kernel which checks the neighborhood in (x - n) and (y - n) direction to be unique)
- [ ] floating point precision bug - there is something wrong with some grid resolutions and the output image. It contains blank pixels at the and of the image. But otherwise if I use floating point numbers there are gaps between the mosaic images)
- [ ] TravisCI configuration for building this app for ~~OS X~~, ~~Linux~~ and maybe Windows? 

# Image Mosaic Wall
This application allows you to create an image based on a bunch of other images. It looks like a mosaic effect. But on detail you can check the following GIF. 

[![Video](https://raw.github.com/scheckmedia/ImageMosaicWall/master/docs/placeholder.jpg)](https://raw.github.com/scheckmedia/ImageMosaicWall/master/docsimw.mp4)


## TODO's
- [ ] Drag & Drop for a single image
- [ ] a better history algorithm (something like a kernel which checks the neighborhood in (x - n) and (y - n) direction to be unique)
- [ ] floating point precision bug - there is something wrong with some grid resolutions and the output image. It contains blank pixels at the and of the image. But otherwise if I use floating point numbers there are gaps between the mosaic images) 
- [ ] TravisCI configuration for building this app for OS X, Linux and maybe Windows? 
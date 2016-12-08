TARGET = thumbnail_test
THUMBNAIL_FILES = main.c resize_base.c resize_gif_img.c resize_png_img.c resize_jpeg_img.c resize_img.c transupp.c exif.c

all: $(TARGET) install

$(TARGET): $(THUMBNAIL_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -ljpeg -lpng -lgif -lexif
	
install:
	mkdir bin
	cp -f $(TARGET) bin/

 
.PHONY: clean
clean:
	rm -f *.o $(TARGET)

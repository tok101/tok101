TARGET = dmthumbnail
THUMBNAIL_FILES = main.c resize_base.c resize_gif_img.c resize_png_img.c resize_jpeg_img.c resize_img.c exif.c 

all: $(TARGET)

$(TARGET): $(THUMBNAIL_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -I./libjpeg/ -ljpeg -lpng -lgif -lexif -rdynamic   -D_GNU_SOURCE
	

 
.PHONY: clean
clean:
	rm -f *.o $(TARGET)

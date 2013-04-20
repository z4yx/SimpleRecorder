struct picture_t {
	unsigned char *buffer;
	struct timeval timestamp;
	unsigned int fourcc;
	int width, height;
};
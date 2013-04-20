int camera_init(struct picture_t *out_info);
int camera_on();
int camera_get_frame(struct picture_t *pic);
int camera_off();
void camera_close();
int output_init();
int output_write(unsigned char *pic, int len);
void output_close();
void osd_print(struct picture_t *pic, const char *str);

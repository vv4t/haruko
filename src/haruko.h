#ifndef HARUKO_H
#define HARUKO_H

#include "buffer.h"

bool haruko_init();
void haruko_bind();
void haruko_update();
bool haruko_should_quit();
void haruko_quit();

buffer_t *haruko_get_image();
buffer_t *haruko_add_buffer();

bool haruko_load_image(GLuint *texture, const char *image_path);
bool haruko_load_cubemap(GLuint *texture, const char *faces[]);

#endif

#include "rrimagelib.h"

/* Read JPEG image from a memory segment */
static void init_source (j_decompress_ptr cinfo) {}

static boolean fill_input_buffer (j_decompress_ptr cinfo)
{
    ERREXIT(cinfo, JERR_INPUT_EMPTY);
return TRUE;
}

static void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
    struct jpeg_source_mgr* src = (struct jpeg_source_mgr*) cinfo->src;

    if (num_bytes > 0) {
        src->next_input_byte += (size_t) num_bytes;
        src->bytes_in_buffer -= (size_t) num_bytes;
    }
}

static void term_source (j_decompress_ptr cinfo) {}

static void jpeg_mem_src (j_decompress_ptr cinfo, void* buffer, long nbytes)
{
    struct jpeg_source_mgr* src;

    if (cinfo->src == NULL) {   /* first time for this JPEG object? */
        cinfo->src = (struct jpeg_source_mgr *)
            (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
            sizeof(struct jpeg_source_mgr));
    }

    src = (struct jpeg_source_mgr*) cinfo->src;
    src->init_source = init_source;
    src->fill_input_buffer = fill_input_buffer;
    src->skip_input_data = skip_input_data;
    src->resync_to_restart = jpeg_resync_to_restart; /* use default method */
    src->term_source = term_source;
    src->bytes_in_buffer = nbytes;
    src->next_input_byte = (JOCTET*)buffer;
}


rrimage *init_rrimage() {
	rrimage *data = (rrimage *) malloc(sizeof(rrimage));
	data->pixels = NULL;
	data->type = TYPE_RRIMAGE_UNSPECIFIED;
	data->quality = 100;

	return data;
}

void free_rrimage(rrimage *data) {
	if (!data) {
		return;
	}

	if (data->pixels) {
		free(data->pixels);
		data->pixels = NULL;
	}

	free(data);
	data = NULL;
}

rrimage* clone_rrimage(rrimage *data) {
	if (!data) {
		return NULL;
	}

	rrimage *result = (rrimage *) malloc(sizeof(rrimage));

	int width = data->width;
	int height = data->height;
	int channels = data->channels;
	int stride = data->stride;

	result->width = width;
	result->height = height;
	result->channels = channels;
	result->stride = stride;
	result->type = data->type;
	result->quality = data->quality;

	if (data->pixels) {
		int size = stride * height;
		result->pixels = (unsigned char *) malloc(size);
		memcpy(result->pixels, data->pixels, size);
	}

	return result;
}

void strip_alpha(rrimage *data) {
	if (data == NULL || data->channels != 4) {
		return;
	}

	int width = data->width;
	int height = data->height;
	int channels = 3;
	int stride = width * channels;
	unsigned char *pixels_temp = data->pixels;

	data->channels = channels;
	data->stride = stride;
	data->pixels = (unsigned char *) malloc(stride * height);

	unsigned char *sptr = pixels_temp;
	unsigned char *dptr = data->pixels;
	int i, j;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			dptr[0] = 255 - (255 - sptr[0]) * sptr[3] / 255;
			dptr[1] = 255 - (255 - sptr[1]) * sptr[3] / 255;
			dptr[2] = 255 - (255 - sptr[2]) * sptr[3] / 255;

			sptr += 4;
			dptr += 3;
		}
	}

	// free original memory
	free(pixels_temp);
}

void my_error_exit(j_common_ptr cinfo) {
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	(*cinfo->err->output_message)(cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}

void my_output_message(j_common_ptr cinfo) {
	char buffer[JMSG_LENGTH_MAX];

	(*cinfo->err->format_message)(cinfo, buffer);
	LOGD("Output_message: %s\n", buffer);
}
void read_jpeg_mem(char *mem_data,int datalen,char* output_data){
	 struct jpeg_decompress_struct cinfo;
	 struct jpeg_error_mgr err;
    	JSAMPARRAY buffer;
 
        cinfo.err = jpeg_std_error(&err);
        jpeg_create_decompress(&cinfo);
        jpeg_mem_src(&cinfo, mem_data, datalen);
        jpeg_read_header(&cinfo, TRUE);
        jpeg_start_decompress(&cinfo);
 
        int bmp_w = cinfo.output_width;
        int bmp_h = cinfo.output_height;
        int bmp_output_cmt = cinfo.output_components;
 
        int row_stride = bmp_w * bmp_output_cmt;
        int row_4_stride = bmp_w * 4;
 
        buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE,row_stride, 1);
 
        int y = 0;
        char* p;
        while (cinfo.output_scanline < bmp_h) {
                jpeg_read_scanlines(&cinfo, buffer, 1);
                p = buffer[0];
                int w = 0;
                int i = 0;
 
                for (; w < row_4_stride; w += 4, i += 3) {
                        *(output_data + row_4_stride * y + w) = *(p + i);
                        *(output_data + row_4_stride * y + w + 1) = *(p + i + 1);
                        *(output_data + row_4_stride * y + w + 2) = *(p + i + 2);
                        *(output_data + row_4_stride * y + w + 3) = 0xff;
                }
 
                y++;
        }
 
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
}

rrimage* read_jpeg_mem_old(char *mem_data,int datalen) {
	if (mem_data == NULL) {
		return NULL;
	}

	rrimage *data = init_rrimage();

	struct jpeg_decompress_struct in;
	struct my_error_mgr in_err;

	in.err = jpeg_std_error(&in_err.pub);
	in_err.pub.error_exit = my_error_exit;
	in_err.pub.output_message = my_output_message;
	if (setjmp(in_err.setjmp_buffer)) {
		jpeg_destroy_decompress(&in);
		return NULL;
	}

	jpeg_create_decompress(&in);
	jpeg_mem_src(&in,mem_data,datalen);
	jpeg_read_header(&in, TRUE);

	jpeg_start_decompress(&in);

	data->width = in.image_width;
	data->height = in.image_height;
	data->channels = 3;
	data->stride = in.image_width * 3 * sizeof(unsigned char);
	data->type = TYPE_RRIMAGE_JPEG;

        
	if(in.out_color_space == JCS_RGB_565){
            LOGD("out_color_space:565");
        } else if(in.out_color_space == JCS_RGBA_8888){
            LOGD("out_color_space:8888");
        } else {
            LOGD("out_color_space:%d",in.out_color_space);
        }

	// 灰度图将会转换为RGB
	data->pixels = (unsigned char *) malloc(data->stride * data->height);
	if (!data->pixels) {
		LOGD("out of memory when read jpeg file...");
		jpeg_destroy_decompress(&in);
		return NULL;
	}

	JSAMPROW row_pointer[1];

	int channels = in.output_components;
	if (channels == 3) {
		while (in.output_scanline < data->height) {
			row_pointer[0] = (&data->pixels[in.output_scanline * data->stride]);
			jpeg_read_scanlines(&in, row_pointer, 1);
		}
	} else if (channels == 1) {
		int i;
		unsigned char *line;
		row_pointer[0] = (unsigned char *) malloc(data->width);
		while (in.output_scanline < data->height) {
			line = &data->pixels[in.output_scanline * data->stride];
			jpeg_read_scanlines(&in, row_pointer, 1);
			for (i = 0; i < data->width; i++, line += 3) {
				line[0] = row_pointer[0][i];
				line[1] = line[0];
				line[2] = line[0];
			}
		}
		free(row_pointer[0]);
	} else {
		LOGD("unsupported jpeg format...channels = %d", channels);
		jpeg_destroy_decompress(&in);
		free_rrimage(data);
		return NULL;
	}

	jpeg_finish_decompress(&in);
	jpeg_destroy_decompress(&in);

	return data;
}

rrimage* read_jpeg(char *file_name) {
	if (!file_name) {
		return NULL;
	}

	FILE *in_file;
	if ((in_file = fopen(file_name, "rb")) == NULL) {
		return NULL;
	}

	rrimage *data = init_rrimage();

	struct jpeg_decompress_struct in;
	struct my_error_mgr in_err;

	in.err = jpeg_std_error(&in_err.pub);
	in_err.pub.error_exit = my_error_exit;
	in_err.pub.output_message = my_output_message;
	if (setjmp(in_err.setjmp_buffer)) {
		jpeg_destroy_decompress(&in);
		fclose(in_file);
		return NULL;
	}

	jpeg_create_decompress(&in);
	jpeg_stdio_src(&in, in_file);
	jpeg_read_header(&in, TRUE);

	jpeg_start_decompress(&in);

	data->width = in.image_width;
	data->height = in.image_height;
	data->channels = 3;
	data->stride = in.image_width * 3 * sizeof(unsigned char);
	data->type = TYPE_RRIMAGE_JPEG;

	// 灰度图将会转换为RGB
	data->pixels = (unsigned char *) malloc(data->stride * data->height);
	if (!data->pixels) {
		LOGD("out of memory when read jpeg file...");
		jpeg_destroy_decompress(&in);
		fclose(in_file);
		return NULL;
	}

	JSAMPROW row_pointer[1];

	int channels = in.output_components;
	if (channels == 3) {
		while (in.output_scanline < data->height) {
			row_pointer[0] = (&data->pixels[in.output_scanline * data->stride]);
			jpeg_read_scanlines(&in, row_pointer, 1);
		}
	} else if (channels == 1) {
		int i;
		unsigned char *line;
		row_pointer[0] = (unsigned char *) malloc(data->width);
		while (in.output_scanline < data->height) {
			line = &data->pixels[in.output_scanline * data->stride];
			jpeg_read_scanlines(&in, row_pointer, 1);
			for (i = 0; i < data->width; i++, line += 3) {
				line[0] = row_pointer[0][i];
				line[1] = line[0];
				line[2] = line[0];
			}
		}
		free(row_pointer[0]);
	} else {
		LOGD("unsupported jpeg format...channels = %d", channels);
		jpeg_destroy_decompress(&in);
		fclose(in_file);
		free_rrimage(data);
		return NULL;
	}

	jpeg_finish_decompress(&in);
	jpeg_destroy_decompress(&in);
	fclose(in_file);

	return data;
}

int write_jpeg(char *file_name, rrimage *data) {
	if (file_name == NULL || data == NULL || data->pixels == NULL) {
		return 0;
	}

	FILE *out_file;
	if ((out_file = fopen(file_name, "wb")) == NULL) {
		return 0;
	}

	struct jpeg_compress_struct out;
	struct my_error_mgr out_err;

	out.err = jpeg_std_error(&out_err.pub);
	out_err.pub.error_exit = my_error_exit;
	if (setjmp(out_err.setjmp_buffer)) {
		jpeg_destroy_compress(&out);
		fclose(out_file);
		remove(file_name);
		return 0;
	}

	jpeg_create_compress(&out);
	jpeg_stdio_dest(&out, out_file);

	out.image_width = data->width;
	out.image_height = data->height;

	if (data->channels == 4) {
		strip_alpha(data);
	}
	out.input_components = data->channels;

	if (out.input_components == 1) {
		out.in_color_space = JCS_GRAYSCALE;
	} else {
		out.in_color_space = JCS_RGB;
	}

	jpeg_set_defaults(&out);
	jpeg_set_quality(&out, data->quality, TRUE);
	jpeg_start_compress(&out, TRUE);

	JSAMPROW row_pointer[1];
	int row_stride = data->width * data->channels;

	while (out.next_scanline < out.image_height) {
		row_pointer[0] = &data->pixels[out.next_scanline * row_stride];
		jpeg_write_scanlines(&out, row_pointer, 1);
	}

	jpeg_finish_compress(&out);
	jpeg_destroy_compress(&out);
	fclose(out_file);

	return 1;
}


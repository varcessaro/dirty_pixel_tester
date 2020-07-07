#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "kmz_core.h"
#include "kmz_gd_2x_image_file.h"
#include "dpc_contrast.h"

int main(int argc, const char * argv[]) {
    FILE * f = NULL;
    FILE * o = NULL;
    if (1 < argc) {
        f = fopen(argv[1], "rb+");
    } else {
        f = stdin;
    }
    if (2 < argc) {
        o = fopen(argv[2], "wb+");
    } else {
        o = stdout;
    }
    
    KmzGd2xImageFile img = {};
    kmz_gd_2x_image_file_status status = kmz_read_gd_2x_image_file(f, &img);
    if (status != OK) {
        fputs(kmz_status_msg(status), stderr);
        fclose(f);
        fclose(o);
        return (int)status;
    }
    
    KmzImage * w = kmz_make_image_from_gd_2x(img);
    dpc_perform_contrast(w);
    memcpy(img.pixels, w->pixels, w->len * sizeof(kmz_color_32));
    
    status = kmz_write_gd_2x_image_file(o, &img);
    if (status != OK) {
        fputs(kmz_status_msg(status), stderr);
    }
    fclose(f);
    fclose(o);
    free(w->pixels);
    free(w);
    free(img.pixels);
    return (int)status;
}


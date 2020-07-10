#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "kmz_core.h"
#include "kmz_gd_2x_image_file.h"
#include "dpc_contrast.h"

struct args_t {
    FILE * i;
    FILE * o;
    KmzArgbColor * colors;
};
typedef struct args_t Args;


size_t has_color(KmzArgbColor c) {
    return c.a == 0 && (c.r > 127 || c.g > 127 || c.b > 127);
}

/**
 * Converts the arguments supplied to a list of argument structs.
 */
int parse_args(const int argc, const char * argv[], Args * args) {
    size_t has_trans = 0, has_dirty = 0, has_dark = 0, has_too_dark = 0;
    KmzArgbColor trans = KmzArgbColor__BLACK, dirty = KmzArgbColor__BLACK, dark = KmzArgbColor__BLACK, too_dark = KmzArgbColor__BLACK;
    for (size_t i = 0; i < argc; i += 2) {
        const char * arg_type = argv[i];
        const char * arg_value = argv[i + 1];
        
        if (strcmp(arg_type, "-i") == 0) {
            args->i = fopen(arg_value, "rb+");
        } else if (strcmp(arg_type, "-o") == 0) {
            args->o = fopen(arg_value, "wb+");
        } else if (strcmp(arg_type, "--trans") == 0) {
            trans = KmzArgbColor__from_hex(arg_value);
            has_trans = has_color(trans);
        } else if (strcmp(arg_type, "--dirty") == 0) {
            dirty = KmzArgbColor__from_hex(arg_value);
            has_dirty = has_color(dirty);
        } else if (strcmp(arg_type, "--dark") == 0) {
            dark = KmzArgbColor__from_hex(arg_value);
            has_dark = has_color(dark);
        } else if (strcmp(arg_type, "--too-dark") == 0) {
            too_dark = KmzArgbColor__from_hex(arg_value);
            has_too_dark = has_color(too_dark);
        } else {
            return -127;
        }
    }
    if (!args->i) {
        args->i = stdin;
    }
    if (!args->o) {
        args->o = stdout;
    }
    if (has_trans && has_dirty && has_dark && has_too_dark) {
        args->colors = calloc(4, sizeof(KmzArgbColor));
        args->colors[0] = trans;
        args->colors[1] = dirty;
        args->colors[2] = dark;
        args->colors[3] = too_dark;
    } else if (has_trans || has_dirty || has_dark || has_too_dark) {
        return -128;
    }
    return 0;
}

int main(int argc, const char * argv[]) {
    Args args = {};
    int result = parse_args(argc - 1, argv + 1, &args);
    
    if (0 != result) {
        return result;
    }
    
    FILE * f = args.i;
    FILE * o = args.o;
    KmzArgbColor * colors = args.colors;
    size_t colors_c = NULL == colors ? 0 : 4;
    
    KmzGd2xImageFile * img = malloc(sizeof(KmzGd2xImageFile));
    kmz_gd_2x_image_file_status status = kmz_read_gd_2x_image_file(f, img);
    if (status != OK) {
        fputs(kmz_status_msg(status), stderr);
        fclose(f);
        fclose(o);
        return (int)status;
    }
    
    KmzImage * w = KmzImage__new_from_gd_2x(img);
    dpc_perform_contrast(w, colors_c, colors);
    memcpy(img->pixels, w->pixels, w->len * sizeof(kmz_color_32));
    
    status = kmz_write_gd_2x_image_file(o, img);
    if (status != OK) {
        fputs(kmz_status_msg(status), stderr);
    }
    fclose(f);
    fclose(o);
    free(w->pixels);
    free(w);
    free(img->pixels);
    free(img);
    free(args.colors);
    return (int)status;
}


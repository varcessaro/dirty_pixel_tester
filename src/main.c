#include <stdio.h>
#include <stdint.h>
#include "dpc_core.h"
#include "dpc_contrast.h"
#include "dpc_gdimage.h"

int main(int argc, const char * argv[]) {
    dpc_file f = NULL;
    if (2 == argc) {
        f = fopen(argv[1], "wb+");
    } else {
        f = stdin;
    }
    
    if (f) {
        struct dpc_gd_2x_header_t header;
        dpc_byte result;
        
        if ((result = dpc_read_gd_2x_header(f, &header))) {
            struct dpc_gd_2x_header_t output_header = {
                .s={
                    .signature=header.s.signature,
                    .width=header.s.width,
                    .height=header.s.height,
                },
                .c={
                    .is_truecolor=0,
                    .header={.truecolor={.transparent=4294967295},},
                },
            };
            dpc_color_32 * body = calloc(header.s.width * header.s.height, sizeof(dpc_color_32));
            dpc_color_32 * output = calloc(header.s.width * header.s.height, sizeof(dpc_color_32));
            if ((result = dpc_read_gd_2x_bytes(f, &header, body))) {
                if (dpc_perform_contrast(header.s.width, header.s.height, 5, &header, body, output)) {
                    dpc_file o = NULL;
                    if (f == stdin) {
                        o = stdout;
                    } else {
                        o = f;
                    }
                    if (o) {
                        if ((result = dpc_write_gd_2x_header(o, &output_header))) {
                            result = dpc_write_gd_2x_bytes(o, &output_header, output);
                        }
                        fclose(o);
                    }
                }
            }
            free(body);
            free(output);
        }
        fclose(f);
        if (1 == result) {
            return 0;
        } else {
            return result + 1;
        }
    } else {
        // ERROR: Failed to load image
        return 1;
    }
}

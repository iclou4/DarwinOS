#include <stdio.h>
#include <stdlib.h>
#include "disk_utility_view.h"

int main(void) {
    int w = DISK_UTILITY_W, h = DISK_UTILITY_H;
    uint32_t *buf = (uint32_t *)calloc((size_t)w * (size_t)h, 4);
    if (!buf) return 1;

    DUState s;
    du_state_init(&s);

    /* Render Normal Main View */
    du_render_state_buf(buf, w, h, w, &s);
    FILE *fp = fopen("/tmp/du_main.ppm", "wb");
    fprintf(fp, "P6\n%d %d\n255\n", w, h);
    for (int i = 0; i < w * h; i++) {
        uint32_t px = buf[i];
        unsigned char r = (px >> 16) & 255;
        unsigned char g = (px >> 8) & 255;
        unsigned char b = px & 255;
        fputc(r, fp); fputc(g, fp); fputc(b, fp);
    }
    fclose(fp);
    printf("Rendered du_main.ppm\n");

    /* Render Partition Sheet */
    s.sheet = DU_SHEET_PARTITION;
    du_render_state_buf(buf, w, h, w, &s);
    fp = fopen("/tmp/du_partition.ppm", "wb");
    fprintf(fp, "P6\n%d %d\n255\n", w, h);
    for (int i = 0; i < w * h; i++) {
        uint32_t px = buf[i];
        unsigned char r = (px >> 16) & 255;
        unsigned char g = (px >> 8) & 255;
        unsigned char b = px & 255;
        fputc(r, fp); fputc(g, fp); fputc(b, fp);
    }
    fclose(fp);
    printf("Rendered du_partition.ppm\n");

    /* Render Erase Sheet */
    s.sheet = DU_SHEET_ERASE;
    du_render_state_buf(buf, w, h, w, &s);
    fp = fopen("/tmp/du_erase.ppm", "wb");
    fprintf(fp, "P6\n%d %d\n255\n", w, h);
    for (int i = 0; i < w * h; i++) {
        uint32_t px = buf[i];
        unsigned char r = (px >> 16) & 255;
        unsigned char g = (px >> 8) & 255;
        unsigned char b = px & 255;
        fputc(r, fp); fputc(g, fp); fputc(b, fp);
    }
    fclose(fp);
    printf("Rendered du_erase.ppm\n");

    /* Render First Aid Sheet */
    s.sheet = DU_SHEET_FIRST_AID;
    du_render_state_buf(buf, w, h, w, &s);
    fp = fopen("/tmp/du_firstaid.ppm", "wb");
    fprintf(fp, "P6\n%d %d\n255\n", w, h);
    for (int i = 0; i < w * h; i++) {
        uint32_t px = buf[i];
        unsigned char r = (px >> 16) & 255;
        unsigned char g = (px >> 8) & 255;
        unsigned char b = px & 255;
        fputc(r, fp); fputc(g, fp); fputc(b, fp);
    }
    fclose(fp);
    printf("Rendered du_firstaid.ppm\n");

    free(buf);
    return 0;
}

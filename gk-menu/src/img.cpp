#include <png.h>
#include <map>
#include <img.h>
#include <cstdio>
#include <lvgl.h>

std::map<std::string, lv_image_dsc_t *> img_map;

const lv_image_dsc_t *get_img(const std::string &fname)
{
    if(img_map.find(fname) != img_map.end())
    {
        return img_map[fname];
    }

    FILE *fp = fopen(fname.c_str(), "rb");
    if(!fp)
        return nullptr;
    
    auto png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if(!png)
    {
        fclose(fp);
        return nullptr;
    }

    auto info = png_create_info_struct(png);
    if(!info)
    {
        fclose(fp);
        return nullptr;
    }

    png_init_io(png, fp);
    png_read_info(png, info);

    auto width = png_get_image_width(png, info);
    auto height = png_get_image_height(png, info);
    auto color_type = png_get_color_type(png, info);
    auto bit_depth = png_get_bit_depth(png, info);

    if(bit_depth == 16)
        png_set_strip_16(png);
    
    if(color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if(png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    if(color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xff, PNG_FILLER_AFTER);
    
    if(color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);
    

    png_read_update_info(png, info);

    auto row_pointers = new png_bytep[height];
    for(unsigned int y = 0; y < height; y++)
        row_pointers[y] = new png_byte[png_get_rowbytes(png, info)];
    
    png_read_image(png, row_pointers);
    fclose(fp);

    // convert to rbg565+alpha
    auto ret = new char[width*height*3];
    for(unsigned int y = 0; y < height; y++)
    {
        auto rp = row_pointers[y];
        for(unsigned int x = 0; x < width; x++)
        {
            auto r = rp[x * 4];
            auto g = rp[x * 4 + 1];
            auto b = rp[x * 4 + 2];
            auto a = rp[x * 4 + 3];

            auto b1 = (b >> 3) | ((g & 0x7) << 5);
            auto b2 = (g >> 5) | (r & 0xf8);
            
            ret[y * width * 3 + x * 3] = b1;
            ret[y * width * 3 + x * 3 + 1] = b2;
            ret[y * width * 3 + x * 3 + 2] = a;
        }
        delete[] row_pointers[y];
    }
    delete[] row_pointers;

    auto id = new lv_image_dsc_t;
    id->header.magic = LV_IMAGE_HEADER_MAGIC;
    id->header.cf = LV_COLOR_FORMAT_ARGB8565;
    id->header.flags = 0;
    id->header.w = width;
    id->header.h = height;
    id->header.stride = width * 3;
    id->data_size = width * height * 3;
    id->data = (const uint8_t *)ret;

    img_map[fname] = id;
    return id;
}

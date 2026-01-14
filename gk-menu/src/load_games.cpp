#include <squirrel.h>
#include <sqstdio.h>
#include <vector>
#include <string>
#include <stdarg.h>
#include <dirent.h>
#include "game.h"

extern std::vector<Game> games;
static HSQUIRRELVM v;

#define ADD_DEFINE(x) \
    sq_pushstring(v, #x, -1); \
    sq_pushinteger(v, x); \
    sq_newslot(v, -3, false);

static void dump_item(int i, bool quote_strings = true)
{
    auto type = sq_gettype(v, i);

    if(i < 0)
        i = sq_gettop(v) + i + 1;

    switch(type)
    {
        case OT_STRING:
            {
                const SQChar *str;
                sq_getstring(v, i, &str);
                if(quote_strings)
                    printf("\"%s\"", str);
                else
                    printf("%s", str);
            }
            break;

        case OT_INTEGER:
            {
                SQInteger val;
                sq_getinteger(v, i, &val);
                printf("%lld", val);
            }
            break;

        case OT_CLOSURE:
            {
                SQInteger nparams, nfree;
                sq_getclosureinfo(v, i, &nparams, &nfree);
                sq_getclosurename(v, i);
                dump_item(-1, false);
                sq_pop(v, 1);
                printf("(%lld)", nparams);

                /*sq_getclosureroot(v, i);
                dump_item(-1);
                sq_pop(v, 1); */        
            }
            break;

        case OT_TABLE:
            {
                printf("{ ");
                sq_pushnull(v);
                while(SQ_SUCCEEDED(sq_next(v, i)))
                {
                    const SQChar *key;
                    dump_item(-2, false);
                    printf("=");
                    dump_item(-1);
                    printf(" ");
                    sq_pop(v, 2);
                }
                sq_pop(v, 1);
                printf("}");
            }
            break;

        case OT_NULL:
            printf("NULL");
            break;

        case OT_NATIVECLOSURE:
            printf("native()");
            break;

        case OT_BOOL:
            {
                SQBool val;
                sq_getbool(v, i, &val);
                if(val) printf("true"); else printf("false");
            }
            break;

        case OT_ARRAY:
            {
                printf("[ ");
                sq_pushnull(v);
                while(SQ_SUCCEEDED(sq_next(v, i)))
                {
                    const SQChar *key;
                    dump_item(-1);
                    printf(", ");
                    sq_pop(v, 2);
                }
                sq_pop(v, 1);
                printf("]");
            }
            break;


        default:
            printf("<UNKNOWN TYPE %d>", type);
            break;
    }
}

static void comp_error(HSQUIRRELVM vm, const SQChar *desc, const SQChar *src, SQInteger line, SQInteger col)
{
    printf("compile error %s\n", desc);
}

static SQInteger file_lexfeedASCII(SQUserPointer file)
{
    int ret;
    char c;
    if( ( ret=fread(&c,sizeof(c),1,(FILE *)file )>0) )
        return c;
    return 0;
}

static int compile_file(HSQUIRRELVM v,const char *filename)
{
    FILE *f=fopen(filename,"rb");
    if(f)
    {
         printf("compile: %d\n", sq_compile(v,file_lexfeedASCII,f,filename,1));
         fclose(f);
         return 1;
    }
    return 0;
}

static SQInteger error_handler(HSQUIRRELVM vm)
{
    printf("error raised: ");
    dump_item(-1, false);
    printf("\n");
    return 0;
}

static void printfunc(HSQUIRRELVM vm, const SQChar *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    vprintf(msg, ap);
    va_end(ap);
}

static void errfunc(HSQUIRRELVM vm, const SQChar *msg, ...)
{
    printf("ERROR: ");
    va_list ap;
    va_start(ap, msg);
    vprintf(msg, ap);
    va_end(ap);
    printf("\n");
}

static bool read_bool(const std::string &name, bool *out)
{
    SQBool val;
    sq_pushstring(v, name.c_str(), name.length());
    auto ret = SQ_SUCCEEDED(sq_get(v, -2));
    if(ret)
    {
        if(SQ_SUCCEEDED(sq_getbool(v, -1, &val)))
            *out = val;
        sq_pop(v, 1);
    }
    return ret;
}

static bool read_int(const std::string &name, int *out)
{
    SQInteger val;
    sq_pushstring(v, name.c_str(), name.length());
    auto ret = SQ_SUCCEEDED(sq_get(v, -2));
    if(ret)
    {
        if(SQ_SUCCEEDED(sq_getinteger(v, -1, &val)))
            *out = val;
        sq_pop(v, 1);
    }
    return ret;
}

static bool read_string(const std::string &name, std::string *out)
{
    const SQChar *val;
    sq_pushstring(v, name.c_str(), name.length());
    auto ret = SQ_SUCCEEDED(sq_get(v, -2));
    if(ret)
    {
        if(SQ_SUCCEEDED(sq_getstring(v, -1, &val)))
            *out = std::string(val);
        sq_pop(v, 1);
    }
    return ret;
}

static bool read_int_array(const std::string &name, std::vector<int> *out)
{
    std::vector<int> val;

    sq_pushstring(v, name.c_str(), name.length());
    auto ret = SQ_SUCCEEDED(sq_get(v, -2));

    if(ret)
    {
        sq_pushnull(v);
        while(SQ_SUCCEEDED(sq_next(v, -2)))
        {
            SQInteger ival;
            auto ret2 = SQ_SUCCEEDED(sq_getinteger(v, -1, &ival));
            if(ret2) val.push_back(ival);
            sq_pop(v, 2);
        }

        *out = val;
    }
    sq_pop(v, 2);

    return ret;
}

static bool read_string_array(const std::string &name, std::vector<std::string> *out)
{
    std::vector<std::string> val;

    sq_pushstring(v, name.c_str(), name.length());
    auto ret = SQ_SUCCEEDED(sq_get(v, -2));

    if(ret)
    {
        sq_pushnull(v);
        while(SQ_SUCCEEDED(sq_next(v, -2)))
        {
            const SQChar *cval;
            auto ret2 = SQ_SUCCEEDED(sq_getstring(v, -1, &cval));
            if(ret2) val.push_back(std::string(cval));
            sq_pop(v, 2);
        }
        sq_pop(v, 2);
        *out = val;
    }

    return ret;
}

static int load_game(const char *fname)
{
    if(compile_file(v, fname) != 1)
    {
        printf("load_game: compile_file failed\n");
        return -1;
    }

    sq_pushroottable(v);
    if(!SQ_SUCCEEDED(sq_call(v, 1, false, true)))
    {
        printf("load_game: sq_call failed\n");
        return -1;
    }
    sq_pop(v, 1);

    return 0;
}

static int parse_game()
{
    if(sq_gettype(v, -1) != OT_TABLE)
    {
        printf("parse_game: not a table\n");
        return -1;
    }

    Game g;
    read_string("name", &g.name);
    read_string("desc", &g.desc);
    read_string("fname", &g.fname);
    read_string("cwd", &g.cwd);
    read_string("img", &g.img);
    read_string("osd", &g.osd);
    read_int("heap_size", (int *)&g.heap_size);
    read_int("stack_size", (int *)&g.stack_size);
    read_int("graphics_texture_size", (int *)&g.graphics_texture_size);
    read_int("screen_w", &g.screen_w);
    read_int("screen_h", &g.screen_h);
    read_int("screen_pf", &g.screen_pf);
    read_int("screen_refresh", &g.screen_refresh);
    read_bool("screen_ignore_vsync", &g.screen_ignore_vsync);
    read_bool("screen_overwritten_each_frame", &g.screen_overwritten_each_frame);

    bool gik = g.keymap.gamepad_is_keyboard != 0;
    read_bool("gamepad_is_keyboard", &gik);
    g.keymap.gamepad_is_keyboard = gik ? 1 : 0;

    bool gim = g.keymap.gamepad_is_mouse != 0;
    read_bool("gamepad_is_mouse", &gim);
    g.keymap.gamepad_is_mouse = gim ? 1 : 0;

    bool gij = g.keymap.gamepad_is_joystick != 0;
    read_bool("gamepad_is_joystick", &gij);
    g.keymap.gamepad_is_joystick = gij ? 1 : 0;

    bool tik = g.keymap.tilt_is_keyboard != 0;
    read_bool("tilt_is_keyboard", &tik);
    g.keymap.tilt_is_keyboard = tik ? 1 : 0;

    bool tij = g.keymap.tilt_is_joystick != 0;
    read_bool("tilt_is_joystick", &tij);
    g.keymap.tilt_is_joystick = tij ? 1 : 0;

    bool jij = g.keymap.joystick_is_joystick != 0;
    read_bool("joystick_is_joystick", &jij);
    g.keymap.joystick_is_joystick = jij ? 1 : 0;

    bool jim = g.keymap.joystick_is_mouse != 0;
    read_bool("joystick_is_mouse", &jij);
    g.keymap.joystick_is_mouse = jij ? 1 : 0;

    bool tim = g.keymap.touch_is_mouse != 0;
    read_bool("touch_is_mouse", &tim);
    g.keymap.touch_is_mouse = tim ? 1 : 0;

    read_string_array("params", &g.args);

    /* Load keymap */
    sq_pushstring(v, "keymap", -1);
    if(SQ_SUCCEEDED(sq_get(v, -2)))
    {
        if(sq_gettype(v, -1) == OT_TABLE || sq_gettype(v, -1) == OT_ARRAY)
        {
            sq_pushnull(v);

            while(SQ_SUCCEEDED(sq_next(v, -2)))
            {
                SQInteger gkk, sc;
                if(SQ_SUCCEEDED(sq_getinteger(v, -1, &sc)) &&
                    SQ_SUCCEEDED(sq_getinteger(v, -2, &gkk)))
                {
                    if(gkk >= 0 && gkk <= GK_NUMKEYS)
                    {
                        g.keymap.gamepad_to_scancode[gkk] = sc;
                    }
                }

                sq_pop(v, 2);
            }

            sq_pop(v, 1);
        }
        sq_pop(v, 1);
    }

    printf("loaded: %s\n", g.name.c_str());

    games.push_back(g);
    return 0;
}

int load_games()
{
    /* Initialize a squirrel vm with our default global objects */

    v = sq_open(1024);

    sq_newclosure(v, error_handler, 0);
    sq_seterrorhandler(v);
    
    sq_setprintfunc(v, printfunc, errfunc);

    sq_setcompilererrorhandler(v, comp_error);
    //sq_seterrorhandler(v);
    sq_pushroottable(v);
    sqstd_register_iolib(v);
    sq_pop(v, 1);

    // set global defines
    sq_pushroottable(v);
    /*sq_pushstring(v, "GK_PIXELFORMAT_RGB565", -1);
    sq_pushinteger(v, 42);
    sq_newslot(v, -3, false);*/
    ADD_DEFINE(GK_PIXELFORMAT_ARGB8888);
    ADD_DEFINE(GK_PIXELFORMAT_RGB565);
    ADD_DEFINE(GK_PIXELFORMAT_RGB888);
    ADD_DEFINE(GK_PIXELFORMAT_ARGB4444);
    ADD_DEFINE(GK_PIXELFORMAT_ARGB1555);
#if __GAMEKID__ >= 4
    ADD_DEFINE(GK_PIXELFORMAT_A4L4);
    ADD_DEFINE(GK_PIXELFORMAT_A8L8);
    ADD_DEFINE(GK_PIXELFORMAT_ABGR8888);
    ADD_DEFINE(GK_PIXELFORMAT_BGR565);
    ADD_DEFINE(GK_PIXELFORMAT_BGRA8888);
    ADD_DEFINE(GK_PIXELFORMAT_L8);
    ADD_DEFINE(GK_PIXELFORMAT_RGBA8888);
    ADD_DEFINE(GK_PIXELFORMAT_XRGB8888);
#endif

    ADD_DEFINE(GK_KEYA);
    ADD_DEFINE(GK_KEYB);
    ADD_DEFINE(GK_KEYX);
    ADD_DEFINE(GK_KEYY);
    ADD_DEFINE(GK_KEYLEFT);
    ADD_DEFINE(GK_KEYRIGHT);
    ADD_DEFINE(GK_KEYUP);
    ADD_DEFINE(GK_KEYDOWN);
    ADD_DEFINE(GK_KEYVOLUP);
    ADD_DEFINE(GK_KEYVOLDOWN);
    ADD_DEFINE(GK_KEYTILTLEFT);
    ADD_DEFINE(GK_KEYTILTRIGHT);
    ADD_DEFINE(GK_KEYTILTUP);
    ADD_DEFINE(GK_KEYTILTDOWN);
    ADD_DEFINE(GK_KEYJOY);
    ADD_DEFINE(GK_KEYJOYDIGILEFT);
    ADD_DEFINE(GK_KEYJOYDIGIRIGHT);
    ADD_DEFINE(GK_KEYJOYDIGIUP);
    ADD_DEFINE(GK_KEYJOYDIGIDOWN);
    ADD_DEFINE(GK_KEYLB);
    ADD_DEFINE(GK_KEYRB);

    ADD_DEFINE(GK_SCANCODE_0);
    ADD_DEFINE(GK_SCANCODE_1);
    ADD_DEFINE(GK_SCANCODE_2);
    ADD_DEFINE(GK_SCANCODE_3);
    ADD_DEFINE(GK_SCANCODE_4);
    ADD_DEFINE(GK_SCANCODE_5);
    ADD_DEFINE(GK_SCANCODE_6);
    ADD_DEFINE(GK_SCANCODE_7);
    ADD_DEFINE(GK_SCANCODE_8);
    ADD_DEFINE(GK_SCANCODE_9);

    ADD_DEFINE(GK_SCANCODE_A);
    ADD_DEFINE(GK_SCANCODE_B);
    ADD_DEFINE(GK_SCANCODE_C);
    ADD_DEFINE(GK_SCANCODE_D);
    ADD_DEFINE(GK_SCANCODE_E);
    ADD_DEFINE(GK_SCANCODE_F);
    ADD_DEFINE(GK_SCANCODE_G);
    ADD_DEFINE(GK_SCANCODE_H);
    ADD_DEFINE(GK_SCANCODE_I);
    ADD_DEFINE(GK_SCANCODE_J);
    ADD_DEFINE(GK_SCANCODE_K);
    ADD_DEFINE(GK_SCANCODE_L);
    ADD_DEFINE(GK_SCANCODE_M);
    ADD_DEFINE(GK_SCANCODE_N);
    ADD_DEFINE(GK_SCANCODE_O);
    ADD_DEFINE(GK_SCANCODE_P);
    ADD_DEFINE(GK_SCANCODE_Q);
    ADD_DEFINE(GK_SCANCODE_R);
    ADD_DEFINE(GK_SCANCODE_S);
    ADD_DEFINE(GK_SCANCODE_T);
    ADD_DEFINE(GK_SCANCODE_U);
    ADD_DEFINE(GK_SCANCODE_V);
    ADD_DEFINE(GK_SCANCODE_W);
    ADD_DEFINE(GK_SCANCODE_X);
    ADD_DEFINE(GK_SCANCODE_Y);
    ADD_DEFINE(GK_SCANCODE_Z);

    ADD_DEFINE(GK_SCANCODE_RETURN);
    ADD_DEFINE(GK_SCANCODE_RETURN2);
    ADD_DEFINE(GK_SCANCODE_TAB);
    ADD_DEFINE(GK_SCANCODE_ESCAPE);
    ADD_DEFINE(GK_SCANCODE_BACKSPACE);
    ADD_DEFINE(GK_SCANCODE_SPACE);
    ADD_DEFINE(GK_SCANCODE_MINUS);
    ADD_DEFINE(GK_SCANCODE_EQUALS);
    ADD_DEFINE(GK_SCANCODE_LEFTBRACKET);
    ADD_DEFINE(GK_SCANCODE_RIGHTBRACKET);
    ADD_DEFINE(GK_SCANCODE_BACKSLASH);
    ADD_DEFINE(GK_SCANCODE_SEMICOLON);
    ADD_DEFINE(GK_SCANCODE_APOSTROPHE);
    ADD_DEFINE(GK_SCANCODE_GRAVE);
    ADD_DEFINE(GK_SCANCODE_COMMA);
    ADD_DEFINE(GK_SCANCODE_PERIOD);
    ADD_DEFINE(GK_SCANCODE_SLASH);
    ADD_DEFINE(GK_SCANCODE_LCTRL);
    ADD_DEFINE(GK_SCANCODE_RCTRL);
    ADD_DEFINE(GK_SCANCODE_LALT);
    ADD_DEFINE(GK_SCANCODE_RALT);
    ADD_DEFINE(GK_SCANCODE_LSHIFT);
    ADD_DEFINE(GK_SCANCODE_RSHIFT);
    ADD_DEFINE(GK_SCANCODE_CAPSLOCK);

    ADD_DEFINE(GK_SCANCODE_F1);
    ADD_DEFINE(GK_SCANCODE_F2);
    ADD_DEFINE(GK_SCANCODE_F3);
    ADD_DEFINE(GK_SCANCODE_F4);
    ADD_DEFINE(GK_SCANCODE_F5);
    ADD_DEFINE(GK_SCANCODE_F6);
    ADD_DEFINE(GK_SCANCODE_F7);
    ADD_DEFINE(GK_SCANCODE_F8);
    ADD_DEFINE(GK_SCANCODE_F9);
    ADD_DEFINE(GK_SCANCODE_F10);
    ADD_DEFINE(GK_SCANCODE_F11);
    ADD_DEFINE(GK_SCANCODE_F12);

    ADD_DEFINE(GK_SCANCODE_PRINTSCREEN);
    ADD_DEFINE(GK_SCANCODE_SCROLLLOCK);
    ADD_DEFINE(GK_SCANCODE_PAUSE);
    ADD_DEFINE(GK_SCANCODE_INSERT);
    ADD_DEFINE(GK_SCANCODE_HOME);
    ADD_DEFINE(GK_SCANCODE_PAGEUP);
    ADD_DEFINE(GK_SCANCODE_PAGEDOWN);
    ADD_DEFINE(GK_SCANCODE_DELETE);
    ADD_DEFINE(GK_SCANCODE_END);
    ADD_DEFINE(GK_SCANCODE_LEFT);
    ADD_DEFINE(GK_SCANCODE_RIGHT);
    ADD_DEFINE(GK_SCANCODE_UP);
    ADD_DEFINE(GK_SCANCODE_DOWN);
    ADD_DEFINE(GK_SCANCODE_NUMLOCKCLEAR);

    ADD_DEFINE(GK_SCANCODE_KP_DIVIDE);
    ADD_DEFINE(GK_SCANCODE_KP_MULTIPLY);
    ADD_DEFINE(GK_SCANCODE_KP_MINUS);
    ADD_DEFINE(GK_SCANCODE_KP_PLUS);
    ADD_DEFINE(GK_SCANCODE_KP_ENTER);
    ADD_DEFINE(GK_SCANCODE_KP_1);
    ADD_DEFINE(GK_SCANCODE_KP_2);
    ADD_DEFINE(GK_SCANCODE_KP_3);
    ADD_DEFINE(GK_SCANCODE_KP_4);
    ADD_DEFINE(GK_SCANCODE_KP_5);
    ADD_DEFINE(GK_SCANCODE_KP_6);
    ADD_DEFINE(GK_SCANCODE_KP_7);
    ADD_DEFINE(GK_SCANCODE_KP_8);
    ADD_DEFINE(GK_SCANCODE_KP_9);
    ADD_DEFINE(GK_SCANCODE_KP_0);
    ADD_DEFINE(GK_SCANCODE_KP_PERIOD);
    ADD_DEFINE(GK_SCANCODE_KP_EQUALS);

    ADD_DEFINE(GK_SCANCODE_AUDIONEXT);
    ADD_DEFINE(GK_SCANCODE_AUDIOPREV);
    ADD_DEFINE(GK_SCANCODE_AUDIOPLAY);
    ADD_DEFINE(GK_SCANCODE_AUDIOSTOP);

    // set ::games member
    sq_pushstring(v, "games", -1);
    sq_newarray(v, 0);
    sq_newslot(v, -3, false);

    // provide the GK version
    sq_pushstring(v, "gk", -1);
    // __GAMEKID__ is not defined to an actual value for v1
#if __GAMEKID__ > 1
    const int gkver = __GAMEKID__;
#else
    const int gkver = 1;
#endif
    sq_pushinteger(v, gkver);
    sq_newslot(v, -3, false);

    sq_pop(v, 1);

    /* Iterate "games" subdirectory and load nut files */
    auto dir = opendir("games");
    if(dir)
    {
        dirent *de;
        while((de = readdir(dir)))
        {
            if(de->d_type == DT_REG)
            {
                auto name = std::string(de->d_name);
                std::string ending = ".nut";
                if(name.length() >= 4 && std::equal(ending.rbegin(), ending.rend(), name.rbegin()))
                {
                    auto fname = "games/" + name;
                    printf("loading %s\n", fname.c_str());
                    load_game(fname.c_str());
                }
            }
        }
        closedir(dir);
    }

    //dump_game("../nuts/castlevania-nes.nut");
    //dump_game("../nuts/test2.nut");

    sq_pushroottable(v);
    sq_pushstring(v, "games", -1);
    if(!SQ_SUCCEEDED(sq_get(v, -2)))
    {
        printf("load_games: no \"games\" member\n");
        return -1;
    }
    if(sq_gettype(v, -1) != OT_ARRAY)
    {
        printf("load_games: \"games\" is not an array\n");
        return -1;
    }

    sq_pushnull(v);
    while(SQ_SUCCEEDED(sq_next(v, -2)))
    {
        parse_game();
        sq_pop(v, 2);
    }
    sq_pop(v, 1);

    sq_close(v);

    return 0;
}
#include <shell.h>
#include <stdio.h>
#include <lvgl.h>
#include <lvgl/src/drivers/lv_drivers.h>
#include <array>
#include <list>
#include <vector>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <string>

auto fnt = &lv_font_unscii_8;

const unsigned int nlines = 26;
const unsigned int ncols = 80;

std::array<std::array<char, ncols>, nlines> scr_lines;
unsigned int cursor_x = 0;
unsigned int cursor_y = 0;
unsigned int input_start_x = 0;
unsigned int input_start_y = 0;



static void *lvgl_thread(void *);
static void *read_sh_thread(void *);
static void key_cb(lv_event_t *);
static void disp_redraw();
static void handle_scroll();

pthread_mutex_t m_lvgl;

lv_obj_t *disp, *ta, *cursor;

int act_stdin, act_stdout, act_stderr;

int main(int argc, char *argv[])
{
    lv_init();

#ifdef __GAMEKID__
    auto display = lv_gk_display_create();
#else
    auto display = lv_x11_window_create("gk console", 640, 480);
    lv_x11_inputs_create(display, nullptr);
#endif

    lv_display_set_default(display);

    ta = lv_textarea_create(lv_screen_active());
    lv_group_focus_obj(ta);
    lv_obj_set_size(ta, 0, 0);

    disp = lv_label_create(lv_screen_active());

    lv_obj_set_size(disp, 640, 480-208-32);
    lv_obj_set_pos(disp, 0, 32);
    lv_obj_set_style_bg_color(disp, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(disp, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(disp, lv_color_white(), 0);
    lv_obj_set_style_text_font(disp, fnt, 0);

    cursor = lv_obj_create(disp);
    lv_obj_set_size(cursor, 2, fnt->line_height);
    lv_obj_set_style_bg_color(cursor, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(cursor, LV_OPA_COVER, 0);
    lv_obj_set_style_outline_width(cursor, 0, 0);
    lv_obj_set_style_border_width(cursor, 0, 0);
    lv_obj_set_style_radius(cursor, 0, 0);
    lv_obj_set_scrollbar_mode(cursor, LV_SCROLLBAR_MODE_OFF);

    // create pipes for console -> sh and sh -> console
    int pipes_console_to_sh[2], pipes_sh_to_console[2];
    if(pipe(pipes_console_to_sh) != 0)
    {
        fprintf(stderr, "failed to create pipe %s\n", strerror(errno));
    }
    if(pipe(pipes_sh_to_console) != 0)
    {
        fprintf(stderr, "failed to create pipe %s\n", strerror(errno));
    }

    act_stdin = dup(STDIN_FILENO);
    act_stdout = dup(STDOUT_FILENO);
    act_stderr = dup(STDERR_FILENO);

    dup2(pipes_console_to_sh[0], STDIN_FILENO);
    dup2(pipes_sh_to_console[1], STDOUT_FILENO);
    dup2(pipes_sh_to_console[1], STDERR_FILENO);

    close(pipes_console_to_sh[0]);
    close(pipes_sh_to_console[1]);

    auto pipe_console_write = pipes_console_to_sh[1];
    auto pipe_console_read = pipes_sh_to_console[0];

    lv_timer_handler();

    // launch read/write threads
    pthread_t pt1, pt2;
    pthread_create(&pt1, nullptr, lvgl_thread, (void *)(intptr_t)pipe_console_write);
    pthread_create(&pt2, nullptr, read_sh_thread, (void *)(intptr_t)pipe_console_read);

    pthread_mutex_init(&m_lvgl, nullptr);

    setvbuf(stdout, nullptr, _IOLBF, BUFSIZ);
    setvbuf(stderr, nullptr, _IONBF, BUFSIZ);

    // launch sh in the main thread
    int sh_main(int argc, const char *argv[], shell_state *sst);
    const char sh[] = "sh";
    const char *sh_argv[] = { sh, nullptr };
    auto ret = sh_main(1, sh_argv, nullptr);

    freopen("/dev/stdin", "r", stdin);
    freopen("/dev/stdout", "w", stdout);
    freopen("/dev/stderr", "w", stderr);

    return ret;
}

void *read_sh_thread(void *p)
{
    auto pipe_console_read = (int)(intptr_t)p;

    while(true)
    {
        char buf[128];
        auto br = read(pipe_console_read, buf, sizeof(buf)-1);
        buf[sizeof(buf)-1] = 0;

        if(br)
        {
            buf[br] = 0;

            // add the text
            for(unsigned int i = 0; i < br; i++)
            {
                auto c = buf[i];
                if(c == '\n')
                {
                    cursor_x = 0;
                    cursor_y++;
                    handle_scroll();
                }
                else
                {
                    scr_lines[cursor_y][cursor_x] = c;
                    cursor_x++;
                    if(cursor_x >= ncols)
                    {
                        cursor_x = 0;
                        cursor_y++;
                        handle_scroll();
                    }
                }
            }

            input_start_x = cursor_x;
            input_start_y = cursor_y;

            pthread_mutex_lock(&m_lvgl);
            disp_redraw();
            pthread_mutex_unlock(&m_lvgl);
        }

    }
}

void *lvgl_thread(void *p)
{
    auto pipe_console_write = (int)(intptr_t)p;
    lv_obj_add_event_cb(ta, key_cb, LV_EVENT_KEY, p);

    while(true)
    {
        pthread_mutex_lock(&m_lvgl);
        auto timer_wait = lv_timer_handler();
        pthread_mutex_unlock(&m_lvgl);

        usleep(timer_wait * 1000);
    }
}

void key_cb(lv_event_t *ev)
{
    auto key = lv_indev_get_key(lv_indev_active());
    auto pipe_console_write = (int)(intptr_t)ev->user_data;

    switch(key)
    {
        case LV_KEY_ENTER:
        case 13:
            // get text up till now
            {
                std::string s;
                for(auto y = input_start_y; y <= cursor_y; y++)
                {
                    auto start_x = (y == input_start_y) ? input_start_x : 0;
                    auto end_x = (y == cursor_y) ? cursor_x : ncols;

                    for(auto x = start_x; x < end_x; x++)
                    {
                        auto c = scr_lines[y][x];
                        if(c == '0' || c == '\n')
                            break;
                        s.push_back(c);
                    }
                }
                s.push_back('\n');

                cursor_x = 0;
                cursor_y++;
                handle_scroll();

                input_start_x = cursor_x;
                input_start_y = cursor_y;

                write(pipe_console_write, s.c_str(), s.size());
            }
            break;

        case LV_KEY_BACKSPACE:
            if((cursor_y > input_start_y) || ((cursor_y == input_start_y) && (cursor_x > input_start_x)))
            {
                if(cursor_x == 0)
                {
                    cursor_y--;
                    cursor_x = 79;
                }
                else
                {
                    cursor_x--;
                }
                scr_lines[cursor_y][cursor_x] = 0;
            }
            break;

        default:
            {
                scr_lines[cursor_y][cursor_x] = key;
                cursor_x++;
                if(cursor_x >= ncols)
                {
                    cursor_x = 0;
                    cursor_y++;
                    handle_scroll();
                }
            }
            break;
    }

    disp_redraw();
}

void disp_redraw()
{
    std::string s;
    bool is_first = true;

    for(const auto &l : scr_lines)
    {
        if(is_first)
            is_first = false;
        else
            s.push_back('\n');
        for(const auto c : l)
        {
            if(c == 0)
                break;
            else
                s.push_back(c);
        }
    }

    lv_label_set_text(disp, s.c_str());

    lv_obj_set_pos(cursor, cursor_x * 8, cursor_y * fnt->line_height);
    lv_obj_invalidate(disp);
}

void handle_scroll()
{
    if(cursor_y >= nlines)
    {
        // scroll lines up
        auto nscroll = cursor_y - 24;
        for(unsigned int from = nscroll; from < nlines; from++)
        {
            scr_lines[from - nscroll] = scr_lines[from];
        }
        for(unsigned int blank = nlines-nscroll; blank < nlines; blank++)
        {
            scr_lines[blank].fill(0);
        }

        cursor_y -= nscroll;

        if(input_start_y < nscroll)
            input_start_y = 0;
        else
            input_start_y -= nscroll;
    }
}

#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "fcntl.h"
#include "string.h"
#include "keyboard.h"
#include "init.h"

int read_line(int fd, char *buf);
int parse_command(char *buf, int *argc, char ***argv);
void run_command(int index, int argc, char **argv);
int shift_left, shift_right, alt_left, alt_right, ctrl_right, ctrl_left;

int main()
{
    int fd = 0;
    unsigned char buf[2048] = {0};
    char path[] = "/keyboard.dev";
    int index = -1;

    current_dir = "/";
    fd = device_openkeyboard(path, 0);

    while (1)
    {
        int argc = 0;
        char **argv = NULL;
        printf("[MINE]$#:");
        memset(buf, 0, 2048);
        read_line(fd, buf);
        printf("\n");

        index = parse_command(buf, &argc, &argv);

        if (index < 0)
            printf("No Command Found!\n");
        else
            run_command(index, argc, argv); // argc,argv
    }

    close(fd);
    while (1)
        ;
    return 0;
}

char *current_dir = NULL;

int cd_command(int argc, char **argv) {}
int ls_command(int argc, char **argv) {}
int pwd_command(int argc, char **argv)
{
    if (current_dir)
        printf("%s\n", current_dir);
    return 1;
}
int cat_command(int argc, char **argv) {}
int touch_command(int argc, char **argv) {}
int rm_command(int argc, char **argv) {}
int mkdir_command(int argc, char **argv) {}
int rmdir_command(int argc, char **argv) {}
int exec_command(int argc, char **argv) {}
int reboot_command(int argc, char **argv) {}

struct buildincmd shell_internal_cmd[] =
    {
        {"cd", cd_command},
        {"ls", ls_command},
        {"pwd", pwd_command},
        {"cat", cat_command},
        {"touch", touch_command},
        {"rm", rm_command},
        {"mkdir", mkdir_command},
        {"rmdir", rmdir_command},
        {"exec", exec_command},
        {"reboot", reboot_command},
};

void run_command(int index, int argc, char **argv)
{
    printf("run_command %s\n", shell_internal_cmd[index].name);
    shell_internal_cmd[index].function(argc, argv);
}

int find_cmd(char *cmd)
{
    int i = 0;
    for (i = 0; i < sizeof(shell_internal_cmd) / sizeof(struct buildincmd); i++)
        if (!strcmp(cmd, shell_internal_cmd[i].name))
            return i;
    return -1;
}

int parse_command(char *buf, int *argc, char ***argv)
{
    int i = 0;
    int j = 0;

    while (buf[j] == ' ')
        j++;

    for (i = j; i < 256; i++)
    {
        if (!buf[i])
            break;
        if (buf[i] != ' ' && (buf[i + 1] == ' ' || buf[i + 1] == '\0'))
            (*argc)++;
    }

    printf("parse_command argc:%d\n", *argc);

    if (!*argc)
        return -1;

    *argv = (char **)malloc(sizeof(char **) * (*argc));

    printf("parse_command argv:%#018lx,*argv:%#018lx\n", argv, *argv);

    for (i = 0; i < *argc && j < 256; i++)
    {
        *((*argv) + i) = &buf[j];

        while (buf[j] && buf[j] != ' ')
            j++;
        buf[j++] = '\0';
        while (buf[j] == ' ')
            j++;
        printf("%s\n", (*argv)[i]);
    }

    return find_cmd(**argv);
}

unsigned char get_scancode(int fd)
{
    unsigned char ret = 0;
    read(fd, &ret, 1);
    return ret;
}

int analysis_keycode(int fd)
{
    unsigned char x = 0;
    int i;
    int key = 0;
    int make = 0;

    x = get_scancode(fd);

    if (x == 0xE1) // pause break;
    {
        key = PAUSEBREAK;
        for (i = 1; i < 6; i++)
            if (get_scancode(fd) != pausebreak_scancode[i])
            {
                key = 0;
                break;
            }
    }
    else if (x == 0xE0) // print screen
    {
        x = get_scancode(fd);

        switch (x)
        {
        case 0x2A: // press printscreen

            if (get_scancode(fd) == 0xE0)
                if (get_scancode(fd) == 0x37)
                {
                    key = PRINTSCREEN;
                    make = 1;
                }
            break;

        case 0xB7: // UNpress printscreen

            if (get_scancode(fd) == 0xE0)
                if (get_scancode(fd) == 0xAA)
                {
                    key = PRINTSCREEN;
                    make = 0;
                }
            break;

        case 0x1d: // press right ctrl

            ctrl_right = 1;
            key = OTHERKEY;
            break;

        case 0x9d: // UNpress right ctrl

            ctrl_right = 0;
            key = OTHERKEY;
            break;

        case 0x38: // press right alt

            alt_right = 1;
            key = OTHERKEY;
            break;

        case 0xb8: // UNpress right alt

            alt_right = 0;
            key = OTHERKEY;
            break;

        default:
            key = OTHERKEY;
            break;
        }
    }

    if (key == 0)
    {
        unsigned int *keyrow = NULL;
        int column = 0;

        make = (x & FLAG_BREAK ? 0 : 1);

        keyrow = &keycode_map_normal[(x & 0x7F) * MAP_COLS];

        if (shift_left || shift_right)
            column = 1;

        key = keyrow[column];

        switch (x & 0x7F)
        {
        case 0x2a: // SHIFT_L:
            shift_left = make;
            key = 0;
            break;

        case 0x36: // SHIFT_R:
            shift_right = make;
            key = 0;
            break;

        case 0x1d: // CTRL_L:
            ctrl_left = make;
            key = 0;
            break;

        case 0x38: // ALT_L:
            alt_left = make;
            key = 0;
            break;
        default:
            if (!make)
                key = 0;
            break;
        }
        if (key)
        {
            if (key == 0x1c)
            {
                key = '\n';
            }
        }
        if (key)
        {
            return key;
        }
    }
    return 0;
}

int read_line(int fd, char *buf)
{
    int key = 0;
    int count = 0;

    while (1)
    {
        key = analysis_keycode(fd);
        if (key == '\n')
        {
            return count;
        }
        else if (key)
        {
            buf[count++] = key;
            printf("%c", key);
        }
        else
            continue;
    }
}

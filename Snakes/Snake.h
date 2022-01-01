/*Change log*/
// keywords are marked by '*'
// version 1.0.0: the start of recording changes
// version 1.1.0: the *"world" is updated to be *soft, thus
//                the world size can be defined by users
// version 1.1.1: use macro to malloc and check
// version 1.1.2: change "BODY" and so on to "DEFAULT_BODYSTL" and so on
//                use *struct to describe *settings(int column, row;)
//                (char body, land, food_style, border, head_style;)
// version 1.1.9: macro to controll cursor
//                find that the user_defined two_dim array's
//                invoking macro is slightly wrong and then repaired
// version 1.2.0: no longer use array "WORLD", just MOVE CURSOR to
//                erase and add char to the console directly
// version 1.2.1: macro to color
// version 1.3.0: use file to store settings
//                optimize the interface(color and so on)
// version 1.3.1: details
// version 1.4.0: bomb mode
// version 1.5.0: debugged and optimized
// version 1.5.2: seperated into two files, some if-else and macro shortened
// version 1.5.3: use jump tables to replace some convertional functions,
//                some functions moved to the hea file
// version 1.5.4: func "reach_border()" deleted

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <Windows.h>
#include <ctype.h>

#define DIRECTORY "C:/GluttonousSnake_UserData.bin"   // route where stores custom settings
#define RECORD_ROUTE "C:/GluttonousSnake_UserRec.bin" // route where stores records

// numbers stand for death reason:
enum _death_reason
{
    Bomb_crash = 1,
    Self_crash = 2,
    Wall_crash = 3
};

// dead cause translator:
const char *dead_cause_transer[4] = {NULL, "Bomb crash", "Self crash", "Wall crash"};

// secial characters' ASCIIs:
enum _spl_chs
{
    Music = 14,
    Eye = 15,
    Lesson = 21,
    Right = 16,
    Left = 17,
    Up = 30,
    Down = 31
};

// numbers stand for color in the color-setting function:
enum _colors
{
    Black,
    Blue,
    Green,
    Lake_blue,
    Red,
    Purple,
    Yellow,
    White,
    Gray,
    Tinge_blue,
    Tinge_green,
    Light_tinge_green,
    Tinge_red,
    Tinge_purple,
    Tinge_yellow,
    Light_white
};
// set color:
#define COLOR(color_selection) SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color_selection);

/*the struct of settings*/
// modes:
enum _mode
{
    Bomb_delay, // controll the rate of generating bombs. value range: 1~infinite
    Bomb_max,   // the maximum of bomb num. value range: 1~infinite
    Bomb_on,    // turn on bomb mode or not. value range: 0 or 1

    Food_delay, // 1~infinite
    Food_max,   // 1~infinite

    Sleep_time // controll the speed that the game runs. value range: 0~infinte
};

// styles of the objects
enum _style_setting
{
    Bodystl,   // snake body
    Bombstl,   // bomb
    Borderstl, // border of map
    Foodstl,   // food
    Headstl,   // snake head
    Landstl    // blanks in map
};

typedef struct _custom_setting
{
    int mode[6]; // mode settings
    COORD size;  // size of map
    int stl[6];  // style settings(used as char)
} custom_setting;
/*end of struct of settings*/

// the struct of game history records:
typedef struct _custom_rec
{
    int death_cause;  // the reason to death
    int snake_length; // final length
} custom_rec;

int selected_serial = 0; // the serial number of a seleced setting
int setting_num = 0;     // the total num of settings
int rec_num = 0;         // the total num of records

custom_setting default_setting;

custom_setting setting; // current  setting
// struct of the file:int selected_serial, int setting_num, struct _custom_setting 1 2 3 ...

custom_rec record; // current record
// struct of rec file: int rec_num, custom_rec 1 2 3 ...

FILE *stream;   // stream of the file that stores settings
FILE *rec_strm; // stream of the file that stores records

// to numeralization the command chars
const int s1[] = {'W', 'A', 'S', 'D'};
#define NUM_DIREC(index) (s1[(index) % 4])

// corresponde between chars and commands
const int s2[] = {0, 1, Left, Up, 4, Right, Down};
#define CHAR_DIREC(index) (s2[(index) % 7])
//'A' % 7 = 2, CHAR_DIREC('A') = Left
//'W' % 7 = 3, CHAR_DIREC('W') = Up
//'D' % 7 = 5, CHAR_DIREC('D') = Right
//'S' % 7 = 6, CHAR_DIREC('S') = Down

// reverse the direction
const int s3[] = {0, 1, 'D', 'S', 4, 'A', 'W'};
#define REVERSED(index) (s3[(index) % 7])

// the character for head is special(defaultly four characters that looks like "up" "left" "down" and "right")
#define PUTC_HEAD() (setting.stl[Headstl] == '$' ? putchar(CHAR_DIREC(stored_command)) : putchar(setting.stl[Headstl]));

// hide/show cursor:
CONSOLE_CURSOR_INFO cursor_info = {1, 0}; // a variety to controll cursor
#define VIEW_CURSOR(visibility)          \
    cursor_info.bVisible = (visibility); \
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);

// a muti-functional checker
#define EXIT_CHECK(pointer, sentence) \
    if ((pointer) == NULL)            \
    {                                 \
        puts(sentence);               \
        puts("Programe will exit.");  \
        system("PAUSE");              \
        exit(EXIT_FAILURE);           \
    }

// open a file, if it doesn't exit, create one
#define F_OPEN(dst_stream, route)                                                                        \
    (dst_stream) = fopen(route, "r+"); /*try to open*/                                                   \
    if ((dst_stream) == NULL)                                                                            \
    {                                                                                                    \
        /*if the file doesn't exit previously*/                                                          \
        (dst_stream) = fopen(route, "w+"); /*create a file*/                                             \
        EXIT_CHECK((dst_stream), "FILE OPEN FAILED.\nMAYBE YOU SHOULD PROVIDE ADMINISTRATOR RIGHTS.\n"); \
    }

// show a title in the interface
#define TITLE(title_str, var)               \
    system("CLS");                          \
    printf("\t\t   " title_str "\n", var);  \
    printf("\n\t\t");                       \
    for (unsigned i = 0; i < 34; i++)       \
    {                                       \
        printf("%c", setting.stl[Bodystl]); \
    }                                       \
    printf("%c\n", setting.stl[Headstl]);   \
    puts("");

// get a value and check it
#define GET_VALUE(format_str, var)       \
    while (scanf(format_str, &var) != 1) \
    {                                    \
        puts(" \aWRONG INPUT!!!");       \
        printf(" Re_enter please.\n ");  \
        setbuf(stdin, NULL);             \
    }                                    \
    setbuf(stdin, NULL);

// give a prompt and then getvalue
#define PROMPT_AND_CHECK(format, var)               \
    printf("\n\t\t   Enter the value:\n\n\t\t   "); \
    GET_VALUE(format, var)

// if the value goes beyond limits, adjust it by force
#define CONFINE(var, operator, limit)                                 \
    if (!(var operator limit))                                        \
    {                                                                 \
        puts("THE VALUE ASSIGNED HAS GONE BEYONGED GIVEN LIMITS!!!"); \
        var = limit;                                                  \
        puts("Now it is adjusted to " #limit);                        \
        printf("\n\t\t   ");                                          \
        system("PAUSE");                                              \
    }

// double confine
#define DBL_CNFN(l_limit, var, r_limit) \
    CONFINE(var, >=, l_limit)           \
    CONFINE(var, <=, r_limit)

// initialize default settings
void initialize_defaultsettings(void)
{
    default_setting.mode[Bomb_delay] = 25;
    default_setting.mode[Bomb_max] = 50;
    default_setting.mode[Bomb_on] = 1;
    default_setting.mode[Food_delay] = 50;
    default_setting.mode[Food_max] = 10;
    default_setting.mode[Sleep_time] = 100;
    default_setting.size.X = 45;        /*alternative aptions: 46 49*/
    default_setting.size.Y = 34;        /*alternative aptions: 92 94*/
    default_setting.stl[Bodystl] = 'O'; /*alternatives: # $ % & * 0 + O o 8*/
    default_setting.stl[Bombstl] = (char)Eye;
    default_setting.stl[Borderstl] = '#';
    default_setting.stl[Foodstl] = (char)Music; /*alternative option:'$'*/
    default_setting.stl[Headstl] = '$';
    default_setting.stl[Landstl] = ' '; /*alternative option: '*'*/
}

// to get the content of selected setting
void get_setting(void)
{
    if (feof(stream))
    {
        setting = default_setting;
        setting_num = 0;
        selected_serial = 0;
    }
    else
    {
        fread(&selected_serial, sizeof(int), 1, stream);
        fread(&setting_num, sizeof(int), 1, stream);
        if (selected_serial == 0)
        {
            setting = default_setting;
        }
        else
        {
            fseek(stream, 2 * sizeof(int) + (selected_serial - 1) * sizeof(custom_setting), SEEK_SET);
            fread(&setting, sizeof(custom_setting), 1, stream);
        }
    }
}

/*menu of settings*/
void setting_menu(BOOL assignablility, int sec_choice)
{
    int const temp_selected = sec_choice;
    int i = 0;
    F_OPEN(stream, DIRECTORY)
    if (assignablility)
    {
        fseek(stream, 2 * sizeof(int) + (sec_choice - 1) * sizeof(custom_setting), SEEK_SET);
        fread(&setting, sizeof(custom_setting), 1, stream);
    }
    else
    {
        setting = default_setting;
    }

Setting_No:
    VIEW_CURSOR((BOOL)0)

    /*interface*/
    TITLE("Setting No.%d", temp_selected)

    printf("Enter:\n");
    printf("\t\t  -1 to  GO BACK.\n");
    printf("\t\t   0 to  ADAPT THIS SETTING.\n"); /*write into file, and copy to selected_serial*/
    printf("\t\t   1 for MODE.\n");
    printf("\t\t   2 for MAP SIZE.\n");
    printf("\t\t   3 for STYLE.\n");
    if (assignablility)
    {
        printf("\t\t   4 to  SAVE changes.\n"); /*write current "setting" into file*/
        printf("\t\t   5 to  DELETE THIS SETTING.\n");
        printf("\n\t\t   Attention:\n\t\t   Don't forget to save.");
        printf("\n\t\t   Or the changes won't be valid.\n\t\t   ");
    }
    else
    {
        puts("\nNotice that the default setting can't be modified.");
        puts("It can only be viewed.");
        puts("So when selecting a specific parameter,");
        puts("which means an attempt to modify its value,");
        puts("the command will be adjusted to '-1', by force.");
        puts("And the interface will return to a previous menu.");
    }

    printf("\n\t\t   ");
    /*end interface*/

    GET_VALUE("%d", sec_choice)

    if (assignablility)
    {
        DBL_CNFN(-1, sec_choice, 5)
    }
    else
    {
        DBL_CNFN(-1, sec_choice, 3)
    }
    switch (sec_choice)
    {
    case -1:
        EXIT_CHECK((void *)(!fclose(stream)), "USERDATA FILE CLOSE FAILED")
        return;
        break;
    case 0:
        selected_serial = temp_selected;
        rewind(stream);
        fwrite(&temp_selected, sizeof(int), 1, stream);
        puts("Adapted.");
        Sleep(1000);
        break;
    case 1:
    {
    MODES:
        VIEW_CURSOR((BOOL)0)

        TITLE("MODES%c", '\0')
        printf("\t\t  -1/ GO BACK                      /-1\n");
        printf("\t\t   0/ BOMB DELAY      (>= 1): %-4d /0\n", setting.mode[Bomb_delay]);
        printf("\t\t   1/ BOMB NUM MAX    (>= 1): %-4d /1\n", setting.mode[Bomb_max]);
        if (setting.mode[Bomb_on] == 1)
        {
            printf("\t\t   2/ BOMB MODE     (0 or 1): ON   /2\n");
        }
        else
        {
            printf("\t\t   2/ BOMB MODE     (0 or 1): OFF  /2\n");
        }
        puts("");
        printf("\t\t   3/ FOOD DELAY      (>= 1): %-4d /3\n", setting.mode[Food_delay]);
        printf("\t\t   4/ FOOD NUM MAX    (>= 1): %-4d /4\n", setting.mode[Food_max]);
        puts("");
        printf("\t\t   5/ GAME SLEEP TIME (>= 0): %-4d /5\n", setting.mode[Sleep_time]);
        printf("\n\t\t   Tips:The bigger the GAME SLEEP TIME is setted, the slower the game runs.\n");
        printf("\n\t\t   ");
        GET_VALUE("%d", sec_choice)

        if (assignablility)
        {
            DBL_CNFN(-1, sec_choice, 5)
        }
        else
        {
            DBL_CNFN(-1, sec_choice, -1)
        }
        if (sec_choice == -1)
        {
            CONFINE(setting.mode[Bomb_delay], >=, 1)
            CONFINE(setting.mode[Bomb_max], >=, 1)
            DBL_CNFN(0, setting.mode[Bomb_on], 1)
            CONFINE(setting.mode[Food_delay], >=, 1)
            CONFINE(setting.mode[Food_max], >=, 1)
            CONFINE(setting.mode[Sleep_time], >=, 0)
            goto Setting_No;
        }
        else
        {
            PROMPT_AND_CHECK("%d", setting.mode[sec_choice])
        }
        goto MODES;
    }
    break;

    case 2:
    {
    MAPSIZE:
        VIEW_CURSOR((BOOL)0)

        TITLE("MAPSIZE%c", '\0')
        printf("\t\t  -1/ GO     BACK              /-1\n");
        printf("\t\t   0/ COLUMN SIZE (9 ~ 46): %d /0\n", setting.size.Y);
        printf("\t\t   1/ ROW    SIZE (9 ~ 92): %d /1\n", setting.size.X);
        printf("\n\t\t   ");
        GET_VALUE("%d", sec_choice)
        if (assignablility)
        {
            DBL_CNFN(-1, sec_choice, 1)
        }
        else
        {
            DBL_CNFN(-1, sec_choice, -1)
        }
        switch (sec_choice)
        {
        case -1:
            goto Setting_No;
            break;
        case 0:
            puts("");
            PROMPT_AND_CHECK("%hd", setting.size.Y)
            DBL_CNFN(9, setting.size.Y, 46)
            break;
        case 1:
            puts("");
            PROMPT_AND_CHECK("%hd", setting.size.X)
            DBL_CNFN(9, setting.size.X, 92)
            break;
        default:
            break;
        }
        goto MAPSIZE;

        break;
    }
    case 3:
    {
    STYLE:
        VIEW_CURSOR((BOOL)0)

        TITLE("STYLE%c", '\0')
        printf("Enter a serial number to choose:\n");
        printf("\t\t  -1/ GO BACK       \t\t/-1\n");
        printf("\t\t   0/ BODYSTYLE:   %c\t\t/0\n", setting.stl[Bodystl]);
        printf("\t\t   1/ BOMBSTYLE:   %c\t\t/1\n", setting.stl[Bombstl]);
        printf("\t\t   2/ BORDERSTYLE: %c\t\t/2\n", setting.stl[Borderstl]);
        printf("\t\t   3/ FOODSTYLE:   %c\t\t/3\n", setting.stl[Foodstl]);
        printf("\t\t   4/ HEADSTYLE:   %c\t\t/4\n", setting.stl[Headstl]);
        printf("\t\t   5/ LANDSTYLE:   %c\t\t/5\n", setting.stl[Landstl]);
        printf("\n\t\t   Tips: if head style is '$', it stands for '%c' '%c' '%c' '%c'.\n", Up, Left, Down, Right);
        printf("\n\t\t   ");
        GET_VALUE("%d", sec_choice)
        if (assignablility)
        {
            DBL_CNFN(-1, sec_choice, 5)
        }
        else
        {
            DBL_CNFN(-1, sec_choice, -1)
        }
        if (sec_choice == -1)
        {
            goto Setting_No;
        }
        else
        {
            PROMPT_AND_CHECK("%c", setting.stl[sec_choice])
        }
        goto STYLE;
    }
    break;
    case 4: /*save*/
        fseek(stream, 2 * sizeof(int) + (temp_selected - 1) * sizeof(custom_setting), SEEK_SET);
        fwrite(&setting, sizeof(custom_setting), 1, stream);
        puts("\n\t\t   SAVED.");
        Sleep(1000);
        break;
    case 5: /*delete*/
        puts("\n\t\t   ARE YOU SURE TO DELETE THIS?");
        puts("\n\t\t   Enter 1 to DELETE.");
        printf("\t\t   Enter 0 to CANCLE THE OPERATION.\n\n\t\t   ");
        GET_VALUE("%d", sec_choice)
        DBL_CNFN(0, sec_choice, 1)
        if (sec_choice == 0)
        {
            puts("\n\t\t   CANCLED.");
            Sleep(1000);
            goto Setting_No;
        }
        while (i < setting_num - selected_serial)
        {
            fseek(stream, 2 * sizeof(int) + (temp_selected + i) * sizeof(custom_setting), SEEK_SET);
            fread(&setting, sizeof(custom_setting), 1, stream);
            fseek(stream, 2 * sizeof(int) + (temp_selected - 1 + i) * sizeof(custom_setting), SEEK_SET);
            fwrite(&setting, sizeof(custom_setting), 1, stream);
            i++;
        }

        if (temp_selected == selected_serial)
        {
            puts("\t\t   The setting currently using is deleted.");
            puts("\t\t   Instead the default setting will be adapted in a moment.");
            puts("\t\t   You can choose to adapt other settings later.");
            system("PAUSE");

            selected_serial = 0;
            rewind(stream);
            fwrite(&selected_serial, sizeof(int), 1, stream);
        }
        else
        {
            fseek(stream, sizeof(int), SEEK_SET);
        }
        setting_num--;
        fwrite(&setting_num, sizeof(int), 1, stream);
        puts("\n\t\t   DELETED.");
        Sleep(1000);
        return;
        break;
    default:
        break;
    }
    goto Setting_No;
}
/*end setting_menu*/

/*main_menu*/
void front_menu(void)
{
    int sec_choice;
    int i = 0;
Welcome:
    VIEW_CURSOR((BOOL)0)

    TITLE("Welcome to Snake!!!%c", '\0')
    printf("\t\t   0/ Start Now      \t\t/0\n");
    printf("\t\t   1/ Configurations \t\t/1\n");
    printf("\t\t   2/ Records        \t\t/2\n");
    printf("\t\t  -1/ Exit Game      \t\t/-1\n");
    printf("\nEnter a serial number to choose:\n\n\t\t   ");
    GET_VALUE("%d", sec_choice)
    DBL_CNFN(-1, sec_choice, 2)

    switch (sec_choice)
    {
    case -1:
        puts("\n\t\t   ARE YOU SURE TO EXIT?");
        puts("\n\t\t   Enter 1 to EXIT  .");
        puts("\t\t   Enter 0 to CANCLE.");
        printf("\n\t\t   ");
        GET_VALUE("%d", sec_choice)
        DBL_CNFN(0, sec_choice, 1)
        if (sec_choice == 0)
        {
            puts("\n\t\t   CANCLED.");
            Sleep(1000);
            goto Welcome;
        }
        puts("\n\t\t   Game will exit in 3 seconds.");
        Sleep(3000);
        exit(EXIT_SUCCESS);
        break;
    case 0:
        return;
        break;
    case 1:
    {
    Configurations:
        VIEW_CURSOR((BOOL)0)

        /*interface*/
        TITLE("Configurations%c", '\0')
        printf("Enter:\n");
        printf("\t\t  -1 to  GO BACK.\n");
        printf("\t\t   0 for DEFAULT SETTING.\n");
        i = 0;
        while (i < setting_num)
        {
            printf("\t\t   %d for THE %dTH SETTING.\n", i + 1, i + 1);
            i++;
        }
        printf("\t\t   %d to  CREATE A NEW SETTING.\n", setting_num + 1);
        /*end interface*/

        printf("\n\n\t\t   Notice:\n\n\t\t   Currently adapting setting: the %dth.\n", selected_serial);
        printf("\n\t\t   ");
        GET_VALUE("%d", sec_choice)
        DBL_CNFN(-1, sec_choice, setting_num + 1)

        if (sec_choice == -1)
        {
            goto Welcome;
        }
        else if (sec_choice == 0)
        {
            /*default settings*/
            setting_menu((BOOL)0, 0);
        }
        else if (sec_choice == setting_num + 1)
        {
            /*create a new setting*/
            puts("\n\t\t   Tips:\n\t\t   In the new setting, every variety is initialized by the default setting.\n");
            printf("\t\t   ");
            system("PAUSE");
            F_OPEN(stream, DIRECTORY)
            fseek(stream, 2 * sizeof(int) + setting_num * sizeof(custom_setting), SEEK_SET);
            fwrite(&default_setting, sizeof(custom_setting), 1, stream);

            setting_num++;
            fseek(stream, sizeof(int), SEEK_SET);
            fwrite(&setting_num, sizeof(int), 1, stream);
            EXIT_CHECK((void *)(!fclose(stream)), "FILE USERDATA CLOSE FAILED")
            setting_menu((BOOL)1, sec_choice);
        }
        else
        {
            /*usual settings*/
            setting_menu((BOOL)1, sec_choice);
        }
        goto Configurations;
    }
    break;
    case 2:
    {
    Records:
        VIEW_CURSOR((BOOL)0)
        TITLE("Records%c", '\0')
        F_OPEN(rec_strm, RECORD_ROUTE)
        (feof(rec_strm)) ? rec_num = 0 : fread(&rec_num, sizeof(int), 1, rec_strm);

        if (rec_num != 0)
        {
            i = 1;
            while (i <= rec_num)
            {
                printf("\t\t   THE %dTH RECORD:\n", i);
                fread(&record, sizeof(custom_rec), 1, rec_strm);
                printf("\t\t   Length    : %10d\n", record.snake_length);
                printf("\t\t   Dead cause: %10s\n\n", dead_cause_transer[record.death_cause]);
                i++;
            }
        }
        else
        {
            puts("\t\t   Empty record.");
        }
        EXIT_CHECK((void *)(!fclose(rec_strm)), "RECFILE CLOSE FAILED");
        printf("\n\n\t\t   ");
        system("PAUSE");
        break;
    }

    default:
        break;
    }
    goto Welcome;
}
/*end main menu*/

void save_log(int len)
{
    int sav_lg;
    printf("\n Save record?.\n\n ");
    printf("\n 0 to SKIP\n 1 to SAVE\n\n ");
    GET_VALUE("%d", sav_lg)
    DBL_CNFN(0, sav_lg, 1)
    if (sav_lg == 1)
    {
        record.snake_length = len;
        F_OPEN(rec_strm, RECORD_ROUTE)
        (feof(rec_strm)) ? rec_num = 0 : fread(&rec_num, sizeof(int), 1, rec_strm);
        rec_num++;
        rewind(rec_strm);
        fwrite(&rec_num, sizeof(int), 1, rec_strm);
        fseek(rec_strm, sizeof(int) + (rec_num - 1) * sizeof(custom_rec), SEEK_SET);
        fwrite(&record, sizeof(custom_rec), 1, rec_strm);

        EXIT_CHECK((void *)(!fclose(rec_strm)), "RECFILE CLOSE FAILED");
        puts("\n SAVED");
        Sleep(1000);
    }
}

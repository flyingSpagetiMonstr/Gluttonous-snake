//GAME PAUSE SAVE CONTINUE
//short overflow
//current lenghth
//confine col and row
//上下左右键移动
//+双人
//To do s maked by repeated '/'
////debug一下无敌模式
//解决输入'￥'等中文字符时的'bug'

/*In the original version(1.0)*/
//food num is not limited

/*Change log*/
/*keywords are marked by '*'*/
//version 1.1.0: the *"world" is updated to be *soft, thus
//               the world size can be defined by users.
//version 1.1.1: use macro to malloc and check.
//version 1.1.2: change "BODY" and so on to "DEFAULT_BODYSTL" and so on.
//               use *struct to describe *settings(int column, row;)
//               (char body, land, food_style, border, head_style;)
//version 1.1.9: macro to controll cursor.
//               find that the user_defined two_dim array's
//               invoking macro is slightly wrong and then repaired.
//version 1.2.0: no longer use array "WORLD", just MOVE CURSOR to
//               erase and add char to the console directly.
//version 1.2.1: macro to color.
//version 1.3.0: use file to store settings.
//               optimize the interface(color and so on).

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include <ctype.h>

#define DIRECTORY "D:/GluttonousSnake_UserData.bin"

#define BLANK 4
//In case that the snake may be genarated too close to the border

#define LAST (snake->capacity - 1)
#define LENGTH (snake->capacity)
#define FOOD_NUM (food->capacity)
#define LATEST_FOOD (food->capacity - 1)

typedef struct _mode
{
    int boom_on;
    int sleep_time;
} mode;

typedef struct _style_setting
{
    char bodyStl;
    char borderStl;
    char foodStl;
    char headStl;
    char landStl;
} style_setting;

typedef struct _custom_setting
{
    mode custom_mode;
    COORD size;
    style_setting stl;
} custom_setting;

typedef struct _soft_array
{
    int capacity;
    COORD array[];
} soft_array;

/*global variables*/
COORD head /*coordinate of head of snake*/;
COORD temp_coord /*a multifunctional variety, may ONLY use in MACRO*/;
soft_array *snake, *food;
char stored_command, command;

int selected_serial = 0, setting_num = 0;
custom_setting default_setting, setting;
FILE *stream;
/*struct of the file:int selected_serial,int setting_num;
  struct _custom_setting 1 2 3 ...*/
/*end global variables*/

/*color set*/
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
#define COLOR(color_selection) \
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color_selection);
/*end color set*/

//set the position of cursor
#define MOVE_CURSOR(parameter_coordinate) \
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (parameter_coordinate));

//hide/show cursor
#define VIEW_CURSOR(visibility)                          \
    CONSOLE_CURSOR_INFO cursor_info = {1, (visibility)}; \
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);

#define COORD_TRANSFORME(original_coord) \
    (temp_coord.X = 1 + 2 * (original_coord.X) + 2, temp_coord.Y = (original_coord.Y) + 1, temp_coord)

#define MOVE_CURSOR_2_END()            \
    temp_coord.Y = setting.size.Y + 1; \
    temp_coord.X = 1;                  \
    MOVE_CURSOR(temp_coord)

#define GET_SETTING()                                                                          \
    fseek(stream, 2 * sizeof(int) + (selected_serial - 1) * sizeof(custom_setting), SEEK_SET); \
    fread(&setting, sizeof(custom_setting), 1, stream);

#define INIT_DEFAULT_SETTING()                                    \
    default_setting.custom_mode.boom_on = 0;                      \
    default_setting.custom_mode.sleep_time = 100;                 \
    default_setting.size.X = 46; /*alternative aptions: 46 49*/   \
    default_setting.size.Y = 23; /*alternative aptions: 92 94*/   \
    default_setting.stl.bodyStl = '~';                            \
    default_setting.stl.borderStl = '.';                          \
    default_setting.stl.foodStl = '`'; /*alternative option:'$'*/ \
    default_setting.stl.headStl = '/';                            \
    default_setting.stl.landStl = ' '; /*alternative option: '*'*/

#define INIT_STREAM()                             \
    stream = fopen(DIRECTORY, "rb+");             \
    if (stream == NULL)                           \
    {                                             \
        stream = fopen(DIRECTORY, "wb+");         \
        if (fclose(stream))                       \
        {                                         \
            EXIT_CHECK(NULL, "FILE CLOSE FAILED") \
        }                                         \
    }                                             \
    stream = fopen(DIRECTORY, "rb+");             \
    EXIT_CHECK(stream, "FILE OPEN FAILED");

#define INIT_SETTING_BY_FILE()                           \
    if (feof(stream))                                    \
    {                                                    \
        setting = default_setting;                       \
    }                                                    \
    else                                                 \
    {                                                    \
        fread(&selected_serial, sizeof(int), 1, stream); \
        fread(&setting_num, sizeof(int), 1, stream);     \
        if (selected_serial == 0)                        \
        {                                                \
            setting = default_setting;                   \
        }                                                \
        else                                             \
        {                                                \
            GET_SETTING()                                \
        }                                                \
    }

#define TITLE(title_str, var)              \
    system("CLS");                         \
    printf("\t\t   " title_str "\n", var); \
    printf("\n\t\t");                      \
    for (size_t i = 0; i < 34; i++)        \
    {                                      \
        printf("%c", setting.stl.bodyStl); \
    }                                      \
    printf("%c\n", setting.stl.headStl);   \
    puts("");

#define EXIT_CHECK(pointer, sentence)                \
    if (pointer == NULL)                             \
    {                                                \
        puts(sentence "!!!");                        \
        puts("Programe will exit after 3 seconds."); \
        Sleep(3000);                                 \
        exit(EXIT_FAILURE);                          \
    }

#define GET_VALUE(format_str, var)       \
    while (scanf(format_str, &var) != 1) \
    {                                    \
        puts(" \aWRONG INPUT!!!");       \
        printf(" Re_enter please.\n ");  \
        setbuf(stdin, NULL);             \
    }                                    \
    setbuf(stdin, NULL);

#define PROMPT_AND_CHECK(prompt, default_value, format, var)                \
    printf("\t\t   Please enter " prompt ": " format "\n ", default_value); \
    GET_VALUE(format, var)

#define CONFINE(var, operator, limit)              \
    if (!(var operator limit))                     \
    {                                              \
        puts("THE VALUE ASSIGNED                   \
              HAS GONE BEYONGED GIVEN LIMITS!!!"); \
        var = limit;                               \
        puts("Now it is adjusted to " #limit);     \
        printf("\n\t\t   ");                       \
        system("PAUSE");                           \
    }

#define DBL_CNFN(l_limit, var, r_limit) \
    CONFINE(var, >=, l_limit)           \
    CONFINE(var, <=, r_limit)

/*In case that these "functions" followed are invoked
only once, I decided to use macro. */

#define INIT_EMERGE()                                \
    {                                                \
        for (int i = 0; i < setting.size.X + 2; i++) \
        {                                            \
            printf("%2c", setting.stl.borderStl);    \
        }                                            \
        printf("\n");                                \
        for (int i = 0; i < setting.size.Y; i++)     \
        {                                            \
            printf("%2c", setting.stl.borderStl);    \
            for (int j = 0; j < setting.size.X; j++) \
            {                                        \
                printf("%2c", setting.stl.landStl);  \
            }                                        \
            printf("%2c\n", setting.stl.borderStl);  \
        }                                            \
        for (int i = 0; i < setting.size.X + 2; i++) \
        {                                            \
            printf("%2c", setting.stl.borderStl);    \
        }                                            \
        puts("\n");                                  \
    }

#define GENERATE_SNAKE()                                           \
    snake = (soft_array *)malloc(sizeof(int) + 2 * sizeof(COORD)); \
    EXIT_CHECK(snake, "ALLOC FAILED")                              \
    LENGTH = 2;                                                    \
    head.X = BLANK + rand() % (setting.size.X - 2 * BLANK);        \
    head.Y = BLANK + rand() % (setting.size.Y - 2 * BLANK);        \
    MOVE_CURSOR(COORD_TRANSFORME(head))                            \
    putchar(setting.stl.bodyStl);                                  \
    snake->array[0] = head;                                        \
    head.Y++;                                                      \
    MOVE_CURSOR(COORD_TRANSFORME(head));                           \
    putchar(setting.stl.headStl);                                  \
    snake->array[1] = head;

//两种写法，也可以比较stored_cammand与command（若用此法，注意初始几步//////////////////////////////////////
#define HEAD_CRAWL()                                                                                                       \
    head = move(head, command);                                                                                            \
    if (command == '\0')                                                                                                   \
    {                                                                                                                      \
        head = move(head, stored_command); /*未输入W、A、S、D之一时,以上一轮的指令移动*/                 \
    }                                                                                                                      \
    else                                                                                                                   \
    {                                                                                                                      \
        if (!memcmp(&head, &(snake->array[LAST - 1]), sizeof(COORD)))                                                      \
        {                                                                                                                  \
            head = move(head, stored_command);                                                                             \
            head = move(head, stored_command); /*If moved back, move twice by the last command to invalidate the operate*/ \
        }                                                                                                                  \
        else                                                                                                               \
        {                                                                                                                  \
            stored_command = command; /*若本次指令生效，则将之存储保留*/                                    \
        }                                                                                                                  \
    }

int overlap(COORD *point, COORD *label, int former_adjustor, int later_adjustor)
{

    for (int i = former_adjustor; i < later_adjustor; i++)
    {
        if (memcmp(point, label + i, sizeof(COORD)) == 0)
        {
            return 1; //point包含于label列
        }
    }
    return 0; //point不含于label列
}

void generate_food(int adjustor)
{
    if (adjustor == 0)
    {
        food = (soft_array *)malloc(sizeof(int) + sizeof(COORD));
        FOOD_NUM = 0;
    }
    else
    {
        if (adjustor == 1)
        {
            food = (soft_array *)realloc(food, sizeof(int) + (FOOD_NUM + 1) * sizeof(COORD));
        }
        else
        {
            EXIT_CHECK(NULL, "WRONG ADJUSTOR")
        }
    }
    EXIT_CHECK(food, "ALLOC FAILED")

    FOOD_NUM++;

    do
    {
        food->array[LATEST_FOOD].X = rand() % setting.size.X;
        food->array[LATEST_FOOD].Y = rand() % setting.size.Y;
    } while (overlap(&(food->array[LATEST_FOOD]), snake->array, 0, LENGTH) || (overlap(&(food->array[LATEST_FOOD]), food->array, 0, FOOD_NUM - 1)));

    MOVE_CURSOR(COORD_TRANSFORME(food->array[LATEST_FOOD]))
    putchar(setting.stl.foodStl);
}

COORD move(COORD point, char instruct)
{
    switch (instruct)
    {
    case 'W':
        point.Y--;
        break;
    case 'A':
        point.X--;
        break;
    case 'S':
        point.Y++;
        break;
    case 'D':
        point.X++;
        break;
    default:
        command = '\0';
        break;
    }
    return point;
}

void crawl(void)
{

    MOVE_CURSOR(COORD_TRANSFORME(snake->array[LAST]))
    putchar(setting.stl.bodyStl);

    if (overlap(&head, food->array, 0, FOOD_NUM))
    {
        //吃到食物时
        FOOD_NUM--;
        snake = (soft_array *)realloc(snake, sizeof(int) + (LENGTH + 1) * sizeof(COORD));
        EXIT_CHECK(snake, "ALLOC_FAILED")
        LENGTH++;
    }
    else
    {
        //没吃到食物时
        MOVE_CURSOR(COORD_TRANSFORME(snake->array[0]))
        putchar(setting.stl.landStl);

        for (int i = 0; i < LAST; i++) //memmove/???////////////////////////////////////////////////////
        {
            snake->array[i] = snake->array[i + 1];
        }
    }

    snake->array[LAST] = head;

    MOVE_CURSOR(COORD_TRANSFORME(head))
    putchar(setting.stl.headStl);
}

int reach_border(void)
{
    if ((head.X >= 0) && (head.X < setting.size.X) && (head.Y >= 0) && (head.Y < setting.size.Y))
    {
        return 0; //safe
    }
    else
    {
        return 1; //OVERSTEP THE BOUNDARY
    }
}

int game_over(void) //重复性->整合//////////////////////////
{

    if (overlap(&head, snake->array, 1, LENGTH - 3))
    {
        puts(" GAME OVER!!!\a");
        puts(" THE SNAKE CRASHED INTO ITSELF!!!");
        return 1;
    }
    if (reach_border())
    {
        puts(" GAME OVER!!!\a");
        puts(" THE SNAKE CRASHED AGAINST THE BORDER OF THE WORLD!!!");
        return 1;
    }
    return 0;
}

/*setting_menu*/
void setting_menu(BOOL assignablility, int sec_choice)
{
    int const temp_selected = sec_choice;
    int i = 0;
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
        puts("And the interface will return to the previous menu.");
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
    case -2:

        break;
    case -1:
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
        printf("\t\t  -1/ GO BACK                /-1\n");

        if (setting.custom_mode.boom_on == 1)
        {
            printf("\t\t   0/ BOOM MODE    : ON      /0\n");
        }
        else
        {
            printf("\t\t   0/ BOOM MODE    : OFF     /0\n");
        }
        printf("\t\t   1/ FPS parameter: %-4d    /1\n", setting.custom_mode.sleep_time);

        printf("\n\t\t   Tips:The bigger the FPS parameter is setted, the slower the game runs.\n");
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
            printf("\n\t\t   Input the assigned value to BOOM MODE.\n");
            printf("\n\t\t   ");
            GET_VALUE("%d", setting.custom_mode.boom_on)
            DBL_CNFN(0, setting.custom_mode.boom_on, 1)
            break;
        case 1:
            printf("\n\t\t   Input the assigned value to FPS parameter.\n");
            printf("\n\t\t   ");
            GET_VALUE("%d", setting.custom_mode.sleep_time)
            CONFINE(setting.custom_mode.sleep_time, >=, 0)
            break;
        default:
            EXIT_CHECK(NULL, "WRONG CHOICE");
            break;
        }
        goto MODES;
    }
    break;

    case 2:
    {
    MAPSIZE:
        VIEW_CURSOR((BOOL)0)

        TITLE("MAPSIZE%c", '\0')
        printf("\t\t  -1/ GO     BACK     \t\t/-1\n");
        printf("\t\t   0/ COLUMN SIZE: %d \t\t/0\n", setting.size.Y);
        printf("\t\t   1/ ROW    SIZE: %d \t\t/1\n", setting.size.X);
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
            PROMPT_AND_CHECK("the num of column.\n\t\t   DEFAULT", default_setting.size.Y, "%hd", setting.size.Y)
            break;
        case 1:
            PROMPT_AND_CHECK("the num of    row.\n\t\t   DEFAULT", default_setting.size.X, "%hd", setting.size.X)
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
        printf("\t\t   0/ BODYSTYLE:   %c\t\t/0\n", setting.stl.bodyStl);
        printf("\t\t   1/ BORDERSTYLE: %c\t\t/1\n", setting.stl.borderStl);
        printf("\t\t   2/ FOODSTYLE:   %c\t\t/2\n", setting.stl.foodStl);
        printf("\t\t   3/ HEADSTYLE:   %c\t\t/3\n", setting.stl.headStl);
        printf("\t\t   4/ LANDSTYLE:   %c\t\t/4\n", setting.stl.landStl);
        printf("\n\t\t   ");
        GET_VALUE("%d", sec_choice)
        if (assignablility)
        {
            DBL_CNFN(-1, sec_choice, 4)
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
            PROMPT_AND_CHECK("the value.\n\t\t   DEFAULT", default_setting.stl.bodyStl, "%c", setting.stl.bodyStl);
            break;
        case 1:
            PROMPT_AND_CHECK("the value.\n\t\t   DEFAULT", default_setting.stl.borderStl, "%c", setting.stl.borderStl);
            break;
        case 2:
            PROMPT_AND_CHECK("the value.\n\t\t   DEFAULT", default_setting.stl.foodStl, "%c", setting.stl.foodStl);
            break;
        case 3:
            PROMPT_AND_CHECK("the value.\n\t\t   DEFAULT", default_setting.stl.headStl, "%c", setting.stl.headStl);
            break;
        case 4:
            PROMPT_AND_CHECK("the value.\n\t\t   DEFAULT", default_setting.stl.landStl, "%c", setting.stl.landStl);
            break;
        default:
            break;
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
    case 5:
        puts("\n\t\t   ARE YOU SURE TO DELETE THIS?");
        puts("\n\t\t   Enter 1 to DELETE.");
        printf("\t\t   Enter 0 to CANCLE THE OPERATION.\n\n\t\t   ");
        GET_VALUE("%d", sec_choice)
        DBL_CNFN(0, sec_choice, 1)
        if (sec_choice == 0)
        {
            puts("CANCLED.");
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
        puts("DELETED.");
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

    TITLE("Welcome to Gluttonous Snake!!!%c", '\0')
    printf("\t\t   0/ Start Now      \t\t/0\n");
    printf("\t\t   1/ Configurations \t\t/1\n");
    printf("\t\t  -1/ Exit Game      \t\t/-1\n");
    // printf("\t\t   2/ Records        /2\n");
    //////////////////////////////////////////////////////////////
    printf("\nEnter a serial number to choose:\n\n\t\t   ");
    GET_VALUE("%d", sec_choice)
    DBL_CNFN(-1, sec_choice, 1)

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
            setting_menu((BOOL)0, 0);
        }
        else if (sec_choice == setting_num + 1)
        {
            /*create a new setting*/
            puts("\t\t   Tips:\n\t\t   In the new setting, every variety is initialized by the default setting.\n");
            printf("\t\t   ");
            system("PAUSE");
            fseek(stream, 2 * sizeof(int) + setting_num * sizeof(custom_setting), SEEK_SET);
            fwrite(&default_setting, sizeof(custom_setting), 1, stream);

            setting_num++;
            fseek(stream, sizeof(int), SEEK_SET);
            fwrite(&setting_num, sizeof(int), 1, stream);

            setting_menu((BOOL)1, sec_choice);
        }
        else
        {
            setting_menu((BOOL)1, sec_choice);
        }
        goto Configurations;
    }
    break;
    // case 2:break;
    default:
        break;
    }
    goto Welcome;
}
/*end main menu*/

int main(void)
{
    INIT_DEFAULT_SETTING()

    INIT_STREAM()

    INIT_SETTING_BY_FILE()

start:
    COLOR(Tinge_green)
    VIEW_CURSOR((BOOL)0)

    front_menu();

    GET_SETTING()
    //in menu system, there may be assignments to setting,
    //however, the "selected_serial" works as a well-preserved flag,
    //Here using it to get the appointed setting.

    stored_command = 'D';

    srand((unsigned int)time(NULL));

    system("CLS");
    INIT_EMERGE();
    GENERATE_SNAKE()
    generate_food(0);

    do
    {
        VIEW_CURSOR((BOOL)0)

        if ((FOOD_NUM == 0) || (!(rand() % 3)))
        {
            generate_food(1);
        }

        if (_kbhit())
        {
            command = toupper(_getch());
        }
        setbuf(stdin, NULL);

        HEAD_CRAWL()
        crawl();

        MOVE_CURSOR_2_END()
        printf("\n Current length: %d.\n", LENGTH);

        if (game_over())
        {
            break;
        }
        Sleep(setting.custom_mode.sleep_time); //可使程序滞留

    } while (1);

    printf("\n Final length: %d.\n\n ", LENGTH);
    free(snake);
    free(food);
    Sleep(1000);
    system("PAUSE");
    goto start;
}
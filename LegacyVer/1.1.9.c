//显示圈圈和方块
//上下左右键移动
//relationship between mapsize and sleep time
//+双人
//+彩色
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
//version 1.2.0: optimize the interface(color and so on).

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include <ctype.h>

/*default settings*/
#define DEFAULT_COLUMN 23 //alternative aptions: 46 49
#define DEFAULT_ROW 46    //alternative aptions: 92 94
//Deleted by version 1.1.2:
//        int column, row;
#define DEFAULT_BODYSTL '~'
#define DEFAULT_LANDSTL ' '   //alternative aption: '*'
#define DEFAULT_FOODSTL '`'   //alternative aption:'￥'
#define DEFAULT_BORDERSTL '.' //好像无法输出'￥'这个符号......///////////////////////日后解决//////////
#define DEFAULT_HEADSTL '/'
/*end default settings*/

//Deleted by version 1.1.2:
//        char body, land, food_style, border, head_style;

#define BLANK 4
//In case that the snake may be genarated too close to the border

#define LAST (snake->capacity - 1)
#define LENGTH (snake->capacity)
#define FOOD_NUM (food->capacity)
#define LATEST_FOOD (food->capacity - 1)

/*version 1.1.2*/

typedef struct _style_setting
{
    char bodyStl;
    char landStl;
    char foodStl;
    char borderStl;
    char headStl;
} style_setting;

typedef struct _custom_setting
{
    COORD size;
    style_setting stl;
    int invinciblility; /////////////////////////////
} custom_setting;

custom_setting setting;
/*end version 1.1.2*/

typedef struct _soft_array
{
    int capacity;
    COORD array[];
} soft_array;

/*version 1.1.0*/
typedef struct _soft_map
{
    COORD size;
    char *map;
} soft_map;

#define WORLD(m, n) \
    (*(world.map + (m)*world.size.Y + (n)))
/*end version 1.1.0*/

COORD head;
soft_array *snake, *food;
char stored_command, command;

#define EXIT_CHECK(pointer, sentence)                \
    if (pointer == NULL)                             \
    {                                                \
        puts(sentence "!!!");                        \
        puts("Programe will exit after 3 seconds."); \
        Sleep(3000);                                 \
        exit(EXIT_FAILURE);                          \
    }

#define PROMPT_AND_CHECK(prompt, default_value, format, var)          \
    printf(" Please enter " prompt ": " format "\n ", default_value); \
    while (scanf(format, &var) != 1)                                  \
    {                                                                 \
        puts(" \aWRONG INPUT!!!");                                    \
        printf(" Re_enter please.\n ");                               \
        setbuf(stdin, NULL);                                          \
    }                                                                 \
    setbuf(stdin, NULL);

#define CONFINE(var, operator, limit)              \
    if (!(var operator limit))                     \
    {                                              \
        puts("THE VALUE OF" #var                   \
             "HAS GONE BEYONGED GIVEN LIMITS!!!"); \
        var = limit;                               \
        puts("Now" #var "is adjusted to" #limit);  \
    }

#define PAVE()                                   \
    for (int i = 0; i < setting.size.X; i++)     \
    {                                            \
        for (int j = 0; j < setting.size.Y; j++) \
        {                                        \
            WORLD(i, j) = setting.stl.landStl;   \
        }                                        \
    }

#define GENERATE_SNAKE()                                           \
    snake = (soft_array *)malloc(sizeof(int) + 2 * sizeof(COORD)); \
    EXIT_CHECK(snake, "ALLOC FAILED")                              \
    LENGTH = 2;                                                    \
    COORD ini_tail;                                                \
    ini_tail.X = BLANK + rand() % (setting.size.X - 2 * BLANK);    \
    ini_tail.Y = BLANK + rand() % (setting.size.Y - 2 * BLANK);    \
    WORLD(ini_tail.X, ini_tail.Y) = setting.stl.bodyStl;           \
    WORLD(ini_tail.X, ini_tail.Y + 1) = setting.stl.headStl;       \
    snake->array[0] = ini_tail;                                    \
    ini_tail.Y++;                                                  \
    head = snake->array[1] = ini_tail;

//规律性较强，可考虑整合//////////////////////////
#define INI_EMERGE                               \
    for (int i = 0; i < setting.size.Y + 2; i++) \
    {                                            \
        printf("%2c", setting.stl.borderStl);    \
    }                                            \
    printf("\n");                                \
    for (int i = 0; i < setting.size.X; i++)     \
    {                                            \
        printf("%2c", setting.stl.borderStl);    \
        for (int j = 0; j < setting.size.Y; j++) \
        {                                        \
            printf("%2c", WORLD(i, j));          \
        }                                        \
        printf("%2c\n", setting.stl.borderStl);  \
    }                                            \
    for (int i = 0; i < setting.size.Y + 2; i++) \
    {                                            \
        printf("%2c", setting.stl.borderStl);    \
    }                                            \
    puts("\n");
#define ERASE_ADD

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

//设置光标位置
#define MOVE_CURSOR(parameter_coordinate) \
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (parameter_coordinate));

//hide/show cursor
#define VIEW_CURSOR(visibility)                          \
    CONSOLE_CURSOR_INFO cursor_info = {1, (visibility)}; \
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);

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

void generate_food(soft_map world, int adjustor)
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

    WORLD(food->array[LATEST_FOOD].X, food->array[LATEST_FOOD].Y) = setting.stl.foodStl;
}

COORD move(COORD point, char instruct)
{
    switch (instruct)
    {
    case 'W':
        point.X--;
        break;
    case 'A':
        point.Y--;
        break;
    case 'S':
        point.X++;
        break;
    case 'D':
        point.Y++;
        break;
    default:
        command = '\0';
        break;
    }
    return point;
}

void crawl(soft_map world)
{

    WORLD(snake->array[LAST].X, snake->array[LAST].Y) = setting.stl.bodyStl;

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
        WORLD(snake->array[0].X, snake->array[0].Y) = setting.stl.landStl;

        for (int i = 0; i < LAST; i++) //memmove/???////////////////////////////////////////////////////
        {
            snake->array[i] = snake->array[i + 1];
        }
    }
    snake->array[LAST] = head;

    WORLD(head.X, head.Y) = setting.stl.headStl;
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

        puts(" GAME OVER!\a");
        puts(" THE SNAKE CRASHED INTO ITSELF!!!");
        return 1;
    }
    if (reach_border())
    {
        puts(" GAME OVER!\a");
        puts(" THE SNAKE CRASHED AGAINST THE BORDER OF THE WORLD!!!");
        return 1;
    }
    return 0;
}

//rename
void color(int c)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c); //设置颜色
}

int main(void)
{
    int invincible_decider, replay, default_settings_decider;

    setting.stl.bodyStl = DEFAULT_BODYSTL;
    setting.stl.borderStl = DEFAULT_BORDERSTL;
    setting.stl.foodStl = DEFAULT_FOODSTL;
    setting.stl.headStl = DEFAULT_HEADSTL;
    setting.stl.landStl = DEFAULT_LANDSTL;

    setting.size.X = DEFAULT_COLUMN;
    setting.size.Y = DEFAULT_ROW;
start:
    PROMPT_AND_CHECK("even number(0) to SET parameters BY YOURSELF or odd number(1) to adapt DEFAULT SETTINGS.\n DEFAULT", 1, "%d", default_settings_decider)
    default_settings_decider = default_settings_decider % 2;
    if (default_settings_decider)
    {
        ;
    }
    else
    {
        PROMPT_AND_CHECK("the num of columns(9 <= column <= 46).\n DEFAULT COLUMN", DEFAULT_COLUMN, "%d", setting.size.X)
        PROMPT_AND_CHECK("the num of rows(9 <= row <= 92).\n DEFAULT ROW", DEFAULT_ROW, "%d", setting.size.Y)
        CONFINE(setting.size.X, >=, 9)
        CONFINE(setting.size.X, <=, 46)
        CONFINE(setting.size.Y, >=, 9)
        CONFINE(setting.size.Y, <=, 92)
        PROMPT_AND_CHECK("a character to represent head_style.\n DEFAULT HEAD", DEFAULT_HEADSTL, "%c", setting.stl.headStl)
        PROMPT_AND_CHECK("a character to represent body.\n DEFAULT BODY", DEFAULT_BODYSTL, "%c", setting.stl.bodyStl)
        PROMPT_AND_CHECK("a character to represent food_style.\n DEFAULT FOOD", DEFAULT_FOODSTL, "%c", setting.stl.foodStl)
        PROMPT_AND_CHECK("a character to represent land.\n DEFAULT LAND", DEFAULT_LANDSTL, "%c", setting.stl.landStl)
        PROMPT_AND_CHECK("a character to represent border.\n DEFAULT BORDER", DEFAULT_BORDERSTL, "%c", setting.stl.borderStl)
    }
    PROMPT_AND_CHECK("even number(0) for INVINCIBLE MODE or odd number(1) for NORMAL.\n DEFAULT", 1, "%d", invincible_decider)
    invincible_decider = invincible_decider % 2;

    soft_map world; //不能放前面，column、row赋值后方可定义world
    world.size.X = setting.size.X;
    world.size.Y = setting.size.Y;
    world.map = (char *)malloc(world.size.X * world.size.Y * sizeof(char));
    EXIT_CHECK(world.map, "ALLOC FAILED")

    stored_command = 'D';

    srand((unsigned int)time(NULL));

    PAVE()
    GENERATE_SNAKE()
    generate_food(world, 0);

    do
    {
        if ((FOOD_NUM == 0) || (!(rand() % 3)))
        {
            generate_food(world, 1);
        }
        system("CLS");
        VIEW_CURSOR((BOOL)0)
        INI_EMERGE;

        if (_kbhit())
        {
            command = toupper(_getch());
        }
        setbuf(stdin, NULL);

        HEAD_CRAWL()

        if (invincible_decider && game_over())
        {
            break;
        }

        printf(" The length is %d now.", LENGTH);

        crawl(world);
#if 0
        Sleep(SLEEP_TIME);//可使程序滞留
#endif
    } while (1);

    printf("\n The final length of the snake is %d.\n", LENGTH);
    free(snake);
    printf(" ARE YOU GOING TO REPLAY OR EXIT THE GAME?\n ");
    VIEW_CURSOR((BOOL)1)
    PROMPT_AND_CHECK("odd number(1) to REPLAY or even number(0) to EXIT.\n DEFAULT", 1, "%d", replay)
    replay = replay % 2;
    if (replay)
    {
        goto start;
    }
    else
    {
        puts(" GAME WILL EXIT AFTER 3 SECONDS");
        Sleep(3000);
        exit(0);
    }
}
#include "Snake.h"

// struct to store a array of coordinates. the num of element is variable
typedef struct _soft_array
{
    int capacity;
    COORD array[];
} soft_array;

// global variaties:
COORD head; // coordinate of the head of snake
COORD tmp;  // a multifunctional variety that works as a media

soft_array *snake, *food, *bomb; // coorinates of these objects

char stored_command, command; // the command that player inputted

#define FOOD_NUM (food->capacity)
#define BOMB_NUM (bomb->capacity)

// set the position of cursor to output:
#define MOVE_CURSOR(parameter_coordinate) SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (parameter_coordinate));

/* in case that the characters are printed by "%2c" on to the screen,
the real coordinate of characters on the screen is not the same as varieties stored.
Thus, sometimes a tranform between the two coordinates is required: */
#define COORD_TRANSFORME(original) (tmp.X = 1 + 2 * (original).X + 2, tmp.Y = (original).Y + 1, tmp)

// move the cursor to the end of the map
#define MOVE_CURSOR_TO_END()    \
    tmp.Y = setting.size.Y + 1; \
    tmp.X = 1;                  \
    MOVE_CURSOR(tmp)

void initialize_map(void);
void initialize_snake(void);

int overlap(COORD *point, COORD *label, int later_adjustor);

soft_array *generate_one(soft_array *generated, soft_array *helper);
COORD move(COORD point, char instruct);
void head_move(void);
void crawl(void);

int game_over(void);
void bomb_anime(COORD bomb_origin, int bomb_times, char direction);

int main(void)
{
    COLOR(Tinge_green)

    system("TITLE Snake"); // set the title of the window

    initialize_defaultsettings();
start:
    F_OPEN(stream, DIRECTORY)
    get_setting();
    EXIT_CHECK((void *)(!fclose(stream)), "FCLOSE FAILED")

    VIEW_CURSOR((BOOL)0)

    front_menu(); // enter the main menu
    F_OPEN(stream, DIRECTORY)
    get_setting();
    // in menu system, setting may get changed,
    // however, the "selected_serial" works as a well-preserved flag,
    // Here use it to get the appointed setting.
    EXIT_CHECK((void *)(!fclose(stream)), "FCLOSE FAILED")
    stored_command = 'D';

    srand((unsigned int)time(NULL));

    system("MODE CON cols=999 lines=999"); // set window to be of max size, and clear screen

    initialize_map();
    initialize_snake();

    food = (soft_array *)malloc(sizeof(int));
    EXIT_CHECK(food, "MALLOC FAILED")
    FOOD_NUM = 0;
    bomb = (soft_array *)malloc(sizeof(int));
    EXIT_CHECK(bomb, "MALLOC FAILED")
    BOMB_NUM = 0;

    do
    {
        VIEW_CURSOR((BOOL)0)

        if (((!(rand() % setting.mode[Food_delay])) || (FOOD_NUM == 0)) && (FOOD_NUM < setting.mode[Food_max]))
        {
            food = generate_one(food, bomb); // generate a food
        }
        if (setting.mode[Bomb_on])
        {
            if (((!(rand() % setting.mode[Bomb_delay])) || (BOMB_NUM == 0)) && (BOMB_NUM < setting.mode[Bomb_max]))
            {
                bomb = generate_one(bomb, food); // generate a bomb
            }
        }

        if (_kbhit()) // if keyboard is hit
        {
            command = toupper(_getch());
        }
        setbuf(stdin, NULL); // clear the buffer

        head_move();
        crawl(); // crawl, by the moved "head"

        MOVE_CURSOR_TO_END()
        printf("\n Current length: %d.\n", snake->capacity);
        record.death_cause = game_over();
        if (record.death_cause)
        {
            puts(" GAME OVER!!!\a");
            break;
        }
        Sleep(setting.mode[Sleep_time]); // controll game pace

    } while (1);
    MOVE_CURSOR_TO_END()
    printf("\n Final length: %d.  \n\n ", snake->capacity);
    save_log(snake->capacity);
    free(snake);
    free(food);
    goto start;
}

// initialize the map
void initialize_map(void)
{
    COLOR(Gray)
    for (int i = 0; i < setting.size.X + 2; i++)
    {
        printf("%2c", setting.stl[Borderstl]);
    }
    printf("\n");
    for (int i = 0; i < setting.size.Y; i++)
    {
        printf("%2c", setting.stl[Borderstl]);
        COLOR(Tinge_green)
        for (int j = 0; j < setting.size.X; j++)
        {
            printf("%2c", setting.stl[Landstl]);
        }
        COLOR(Gray)
        printf("%2c\n", setting.stl[Borderstl]);
    }
    for (int i = 0; i < setting.size.X + 2; i++)
    {
        printf("%2c", setting.stl[Borderstl]);
    }
    COLOR(Tinge_green)
    puts("\n");
}

// initialize the snake
void initialize_snake(void)
{
    snake = (soft_array *)malloc(sizeof(int) + 2 * sizeof(COORD));
    EXIT_CHECK(snake, "ALLOC FAILED")
    snake->capacity = 2;
    head.X = 4 + rand() % (setting.size.X - 2 * 4);
    head.Y = 4 + rand() % (setting.size.Y - 2 * 4);
    MOVE_CURSOR(COORD_TRANSFORME(head))
    COLOR(White)
    putchar(setting.stl[Bodystl]);
    COLOR(Tinge_green)
    snake->array[0] = head;
    head.Y++;
    MOVE_CURSOR(COORD_TRANSFORME(head));
    COLOR(Light_white)
    PUTC_HEAD()
    COLOR(Tinge_green)
    snake->array[1] = head;
}

// check whether the point's coordinate is included in the "label"(the start of a array of coordinates)
int overlap(COORD *point, COORD *label, int later_adjustor)
{
    int i = 0;
    while (i < later_adjustor)
    {
        if (memcmp(point, label + i, sizeof(COORD)) == 0)
        {
            return i + 1; //  if point is included in array label, return the serial number of the overlaped point
        }
        i++;
    }
    return 0; // point is not included in array label
}

// generator of food and bomb
soft_array *generate_one(soft_array *generated, soft_array *helper)
{
    int a, b, c, flag;
    flag = (generated == food) ? 1 : 0;

    generated = (soft_array *)realloc(generated, sizeof(int) + (generated->capacity + 1) * sizeof(COORD));
    generated->capacity++;

    do
    {
        generated->array[generated->capacity - 1].X = rand() % setting.size.X;
        generated->array[generated->capacity - 1].Y = rand() % setting.size.Y;

        // overlap check:
        a = overlap(&(generated->array[generated->capacity - 1]), snake->array, snake->capacity);
        b = overlap(&(generated->array[generated->capacity - 1]), generated->array, generated->capacity - 1);
        c = overlap(&(generated->array[generated->capacity - 1]), helper->array, helper->capacity);

    } while (a || b || c);

    MOVE_CURSOR(COORD_TRANSFORME(generated->array[generated->capacity - 1]))

    if (flag == 1)
    {
        // print food
        COLOR(Lake_blue)
        putchar(setting.stl[Foodstl]);
        COLOR(Tinge_green)
    }
    else if (flag == 0)
    {
        // print bomb
        COLOR(Tinge_red)
        putchar(setting.stl[Bombstl]);
        COLOR(Tinge_green)
    }

    return generated;
}

// move the coordinate by instruct
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

// move the "head" of the sanke by instructs
void head_move(void)
{
    if (command == REVERSED(stored_command)) // if the command means going backward
    {
        head = move(head, stored_command);
    }
    else
    {
        head = move(head, command);
        if (command == '\0')
        {
            head = move(head, stored_command); // When input is not one of W¡¢A¡¢S¡¢D, move by the last valid instruction
        }
        else
        {
            stored_command = command; // if the command take effect, store it
        }
    }
}

// snake crawl
void crawl(void)
{
    int food_ate; // the serial number of the food ate just now

    MOVE_CURSOR(COORD_TRANSFORME(snake->array[snake->capacity - 1]))
    COLOR(White)
    putchar(setting.stl[Bodystl]);
    COLOR(Tinge_green)
    food_ate = overlap(&head, food->array, FOOD_NUM);
    if (food_ate)
    {
        // when got a food, delete the coordinate of the food ate
        memmove(food->array + food_ate - 1, food->array + food_ate, (food->capacity - food_ate) * sizeof(COORD));

        snake->capacity++;
        snake = (soft_array *)realloc(snake, sizeof(int) + snake->capacity * sizeof(COORD));
        EXIT_CHECK(snake, "ALLOC_FAILED")

        food->capacity--;
        food = (soft_array *)realloc(food, sizeof(int) + food->capacity * sizeof(COORD));
        EXIT_CHECK(food, "ALLOC_FAILED")
    }
    else
    {
        // when didn't
        MOVE_CURSOR(COORD_TRANSFORME(snake->array[0]))

        putchar(setting.stl[Landstl]);

        // delete the coordinate of snake tail
        memmove(snake->array, snake->array + 1, (snake->capacity - 1) * sizeof(COORD));
    }

    // add the coordinate of snake head
    snake->array[snake->capacity - 1] = head;
    MOVE_CURSOR(COORD_TRANSFORME(head))
    COLOR(Light_white)
    PUTC_HEAD()
    COLOR(Tinge_green)
}

// whether game is over or not
int game_over(void)
{
    // meet bomb
    if (overlap(&head, bomb->array, BOMB_NUM))
    {

        MOVE_CURSOR(COORD_TRANSFORME(head))

        putchar('*');
        bomb_anime(head, 10, '\0'); // alternative value of bomb times: 8
        // There's "COLOR" in "bomb_anime"
        COLOR(Tinge_green)

        // MOVE_CURSOR(stored_coord);
        puts(" THE SNAKE CRASHED INTO A BOMBER!!!");
        return Bomb_crash;
    }
    // meet self
    if (overlap(&head, snake->array, snake->capacity - 1))
    {
        puts(" THE SNAKE CRASHED INTO ITSELF!!!");
        return Self_crash;
    }
    // meet border
    if ((head.X == -1) || (head.X == setting.size.X) || (head.Y == -1) || (head.Y == setting.size.Y))
    {
        puts(" THE SNAKE CRASHED AGAINST THE BORDER OF THE WORLD!!!");
        return Wall_crash;
    }
    return 0;
}

// generate anime when the snake meets with a bomb
void bomb_anime(COORD bomb_origin, int bomb_times, char direction)
{
    if (bomb_times == 0)
    {
        return;
    }

    COORD offsetted;
    bomb_times--;
    MOVE_CURSOR(COORD_TRANSFORME(bomb_origin))

    COLOR(Tinge_red)
    putchar('.');
    COLOR(Red)
    for (int i = 0; i < 4; i++)
    {
        if (direction != NUM_DIREC(i))
        {
            offsetted = move(bomb_origin, NUM_DIREC(i));
            MOVE_CURSOR(COORD_TRANSFORME(offsetted))
            putchar('*');
            if ((rand() % 4 == 0) || bomb_times > 5) // // alternative value that bomb times > : 3
            {
                bomb_anime(offsetted, bomb_times, NUM_DIREC(i + 2));
            }
        }
    }
}
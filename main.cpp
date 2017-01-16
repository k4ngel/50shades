#include <iostream>
#include <pthread.h>
#include <vector>
#include <ncurses.h>
#include <unistd.h>
#include <queue>

#define SUPPLIER_CAPACITY 20
#define BOOKSTORE_CAPACITY 20

#define PRINTING_DURATION 1000000
#define LOADING_DURATION 2000000
#define UNLOADING_DURATION 5000000
#define TRAVEL_DURATION 10000000
#define SELL_DURATION 1000000
#define FRAME_DURATION 300000

struct bookstores_data_t
{
    int id;
    int books;
    int readers;
    bool is_opened;
    bool in_queue;
};

struct suppliers_data_t
{
    int id;
    int books;
    int destination;
    int status;
};
enum
{
    TO_BOOKSTORE = 0,
    FROM_BOOKSTORE,
    IN_QUEUE,
    LOADING,
    UNLOADING
};
std::string statuses[] =
        {
                "Do ksiegarni",
                "Z ksiegarni",
                "Kolejka",
                "Zaladunek",
                "Wyladunek"
        };

int result, bookstores, readers, suppliers;

std::vector<bookstores_data_t> bookstores_data;
std::vector<suppliers_data_t> suppliers_data;
std::queue<int> bookstores_queue;
int printing_office_books;

bool queues;

pthread_cond_t printing_office_cond;
pthread_cond_t bookstores_queue_cond;
pthread_mutex_t printing_office_mutex;
std::vector<pthread_mutex_t> bookstores_mutex;
pthread_mutex_t bookstores_queue_mutex;

std::vector<pthread_t> bookstores_threads;
std::vector<pthread_t> suppliers_threads;
pthread_t printing_office_thread;



void* printing_office(void* arg)
{
    while (queues)
    {
        usleep(PRINTING_DURATION);
        pthread_mutex_lock(&printing_office_mutex);
        ++printing_office_books;
        pthread_cond_signal(&printing_office_cond);
        pthread_mutex_unlock(&printing_office_mutex);
    }

    pthread_exit(NULL);
}

void *supplier(void *arg)
{
    int supplier_id = *((int *) arg);
    free(arg);
    suppliers_data[supplier_id].id = supplier_id;
    suppliers_data[supplier_id].books = 0;
    suppliers_data[supplier_id].status = IN_QUEUE;
    int books = 0;
    //int destination;
    while (queues)
    {
        if (suppliers_data[supplier_id].books == 0)
        {
            suppliers_data[supplier_id].status = IN_QUEUE;
            pthread_mutex_lock(&printing_office_mutex);
            if (printing_office_books > 0)
            {
                suppliers_data[supplier_id].status = LOADING;
                usleep(LOADING_DURATION);
                if (printing_office_books > SUPPLIER_CAPACITY)
                {
                    suppliers_data[supplier_id].books = SUPPLIER_CAPACITY;
                    printing_office_books -= SUPPLIER_CAPACITY;
                }
                else
                {
                    suppliers_data[supplier_id].books = printing_office_books;
                    printing_office_books = 0;
                }
            }
            else
            {
                pthread_cond_wait(&printing_office_cond, &printing_office_mutex);
            }
            pthread_mutex_unlock(&printing_office_mutex);
        }

        if (suppliers_data[supplier_id].books > 0)
        {
            while (true)
            {
                pthread_mutex_lock(&bookstores_queue_mutex);
                if (!bookstores_queue.empty())
                {
                    suppliers_data[supplier_id].destination = bookstores_queue.front();
                    bookstores_queue.pop();
                    pthread_mutex_unlock(&bookstores_queue_mutex);
                    break;
                }
                else
                {
                    pthread_mutex_unlock(&bookstores_queue_mutex);
                    pthread_cond_wait(&bookstores_queue_cond, &bookstores_queue_mutex);
                }
            }


            // go to bookstore
            suppliers_data[supplier_id].status = TO_BOOKSTORE;
            usleep(TRAVEL_DURATION);

            suppliers_data[supplier_id].status = UNLOADING;
            pthread_mutex_lock(&bookstores_mutex[suppliers_data[supplier_id].destination]);
            bookstores_data[suppliers_data[supplier_id].destination].is_opened = false;
            usleep(UNLOADING_DURATION);
            bookstores_data[suppliers_data[supplier_id].destination].books = suppliers_data[supplier_id].books;
            suppliers_data[supplier_id].books = 0;
            bookstores_data[suppliers_data[supplier_id].destination].is_opened = true;
            bookstores_data[suppliers_data[supplier_id].destination].in_queue = false;
            pthread_mutex_unlock(&bookstores_mutex[suppliers_data[supplier_id].destination]);

            // go back
            suppliers_data[supplier_id].status = FROM_BOOKSTORE;
            usleep(TRAVEL_DURATION);
        }
    }

    pthread_exit(NULL);
}

void *bookstore(void* arg)
{
    int bookstore_id = *((int *) arg);
    free(arg);
    bookstores_data[bookstore_id].id = bookstore_id;
    bookstores_data[bookstore_id].books = 0;
    bookstores_data[bookstore_id].is_opened = true;
    bookstores_data[bookstore_id].in_queue = false;
    bool any_queue;
    while (queues)
    {
        if (bookstores_data[bookstore_id].books == 0)
        {
            if (bookstores_data[bookstore_id].in_queue)
            {
                usleep(1000);
            }
            else
            {
                pthread_mutex_lock(&bookstores_queue_mutex);
                bookstores_queue.push(bookstore_id);
                pthread_cond_signal(&bookstores_queue_cond);
                bookstores_data[bookstore_id].in_queue = true;
                pthread_mutex_unlock(&bookstores_queue_mutex);
            }
        }
        else
        {
            if (bookstores_data[bookstore_id].readers > 0)
            {
                usleep(SELL_DURATION);

                pthread_mutex_lock(&bookstores_mutex[bookstore_id]);
                --bookstores_data[bookstore_id].books;
                pthread_mutex_unlock(&bookstores_mutex[bookstore_id]);

                --bookstores_data[bookstore_id].readers;
            }
            else
            {
                any_queue = false;
                for (auto const& single_bookstore: bookstores_data)
                {
                    if (single_bookstore.readers > 0)
                    {
                        any_queue = true;
                        break;
                    }
                }
                if (!any_queue)
                {
                    queues = any_queue;
                }
            }
        }
    }
    pthread_exit(NULL);
}

void drawer()
{
    char echo = ' ';
    int y = 0, x = 0, max_x = 0, max_y = 0;

    getmaxyx(stdscr, max_y, max_x);

    while (true)
    {
        y = 0;

        move(y++, 0);
        sprintf(&echo, "Dostawcow: %d, ksiegarni: %d, klientow: %d\n", suppliers, bookstores, readers);
        printw(&echo);

        move(y++, 0);
        sprintf(&echo, "W drukarni czeka %d ksiazek, nowa powstaje co %d s\n", printing_office_books, PRINTING_DURATION/1000000);
        printw(&echo);
        for (auto const & supplier : suppliers_data)
        {
            move(y++, 0);
            sprintf(&echo, "Dostawca nr %d: %s, cel: %d, ksiazek: %d\n", supplier.id, statuses[supplier.status].c_str(), supplier.destination, supplier.books);
            printw(&echo);
        }
        for (auto const & bookstore : bookstores_data)
        {
            move(y++,0);
            sprintf(&echo, "Ksiegarnia nr %d: %d ksiazek, %d klientow\n", bookstore.id, bookstore.books, bookstore.readers);
            printw(&echo);
        }

        refresh();


        usleep(FRAME_DURATION);
    }
}

void init()
{
    printing_office_books = 0;
    queues = true;
    int readers_this, readers_per_bookstore, readers_left = readers;
    readers_per_bookstore = readers/bookstores;
    pthread_t thread;
    pthread_mutex_t mutex;

    pthread_cond_init(&bookstores_queue_cond, NULL);
    pthread_cond_init(&printing_office_cond, NULL);
    pthread_mutex_init(&bookstores_queue_mutex, NULL);
    pthread_mutex_init(&printing_office_mutex, NULL);

    result = pthread_create(&printing_office_thread, NULL, printing_office, NULL);
    for (int i = 0; i < bookstores; ++i)
    {
        if (i == bookstores - 1)
        {
            readers_this = readers_left;
        }
        else
        {
            readers_this = readers_per_bookstore;
        }
        readers_left -= readers_this;

        pthread_mutex_init(&mutex, NULL);
        bookstores_mutex.push_back(mutex);

        int *arg = (int*) malloc(sizeof(*arg));
        *arg = i;
        bookstores_data_t data = {
                i,
                0,
                readers_this,
                false,
                false
        };
        bookstores_data.push_back(data);
        result = pthread_create(&thread, NULL, bookstore, arg);
        bookstores_threads.push_back(thread);
    }
    for (int i = 0; i < suppliers; ++i)
    {
        int *arg = (int*) malloc(sizeof(*arg));
        *arg = i;
        suppliers_data_t data = {
                i,
                0,
                0,
                IN_QUEUE
        };
        suppliers_data.push_back(data);
        result = pthread_create(&thread, NULL, supplier, arg);
        suppliers_threads.push_back(thread);
    }
}

void destroy()
{
    pthread_cond_destroy(&bookstores_queue_cond);
    pthread_cond_destroy(&printing_office_cond);
    pthread_mutex_destroy(&bookstores_queue_mutex);
    pthread_mutex_destroy(&printing_office_mutex);

    for (int i = 0; i < bookstores; ++i)
    {
        pthread_join(bookstores_threads[i], NULL);
        pthread_mutex_destroy(&bookstores_mutex[i]);
    }
    for (int i = 0; i < suppliers; ++i)
    {
        pthread_join(suppliers_threads[i], NULL);
    }
}

int main(int argc, char* argv[]) {
    std::cin.get();
    if (argc < 4)
    {
        std::cout <<
        "Niepoprawna liczba argumentow. Prawidlowe uzycie: ./projekt_so LICZBA_DOSTAWCOW LICZBA_KSIEGARNI LICZBA_KLIENTOW" <<
        std::endl;
        return 1;
    }
    suppliers = atoi(argv[1]);
    bookstores = atoi(argv[2]);
    readers = atoi(argv[3]);

    if (suppliers < 1 || bookstores < 1 || readers < 1)
    {
        std::cout << "Zarowno dostawcow, ksiegarni jak i klientow powinno byc wiecej niz 0 :)" << std::endl;
        return 1;
    }

    initscr();
    noecho();
    init();
    drawer();
    destroy();
    endwin();

    return 0;
}
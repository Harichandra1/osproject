#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_SHOWTIMES 5
#define MAX_TICKETS_PER_SHOWTIME 10
#define TIME_WINDOW 600

typedef struct {
    int showtime_id;
    int available_tickets;
    pthread_mutex_t showtime_mutex;
    time_t lock_time;
} Showtime;

Showtime showtimes[MAX_SHOWTIMES];

void initializeShowtimes() {
    for (int i = 0; i < MAX_SHOWTIMES; ++i) {
        showtimes[i].showtime_id = i + 1;
        showtimes[i].available_tickets = MAX_TICKETS_PER_SHOWTIME;
        pthread_mutex_init(&showtimes[i].showtime_mutex, NULL);
        showtimes[i].lock_time = 0;
    }
}

int reserveTickets(int showtime_id, int num_tickets) {
    Showtime *showtime = &showtimes[showtime_id - 1];

    pthread_mutex_lock(&showtime->showtime_mutex);

    if (showtime->lock_time == 0 && showtime->available_tickets >= num_tickets) {
        showtime->lock_time = time(NULL);

        sleep(2);

        showtime->available_tickets -= num_tickets;

        pthread_mutex_unlock(&showtime->showtime_mutex);

        return 1;
    } else {
        pthread_mutex_unlock(&showtime->showtime_mutex);
        return 0;
    }
}

void releaseShowtime(int showtime_id) {
    Showtime *showtime = &showtimes[showtime_id - 1];

    pthread_mutex_lock(&showtime->showtime_mutex);

    showtime->lock_time = 0;

    pthread_mutex_unlock(&showtime->showtime_mutex);
}

void checkAndReleaseExpiredLocks() {
    time_t current_time = time(NULL);

    for (int i = 0; i < MAX_SHOWTIMES; ++i) {
        pthread_mutex_lock(&showtimes[i].showtime_mutex);

        if (showtimes[i].lock_time != 0 && current_time - showtimes[i].lock_time > TIME_WINDOW) {
            showtimes[i].lock_time = 0;
            printf("Showtime %d lock released due to timeout.\n", showtimes[i].showtime_id);
        }

        pthread_mutex_unlock(&showtimes[i].showtime_mutex);
    }
}

int main() {
    initializeShowtimes();

    // Simulate multiple customers attempting to reserve tickets
    for (int i = 0; i < 3; ++i) {
        int showtime_id, num_tickets;

        // Get user input for showtime and number of tickets
        printf("Enter showtime ID (1-%d): ", MAX_SHOWTIMES);
        scanf("%d", &showtime_id);

        printf("Enter the number of tickets to reserve: ");
        scanf("%d", &num_tickets);

        int reservation = reserveTickets(showtime_id, num_tickets);

        if (reservation) {
            printf("Customer reserved %d tickets for Showtime %d.\n", num_tickets, showtime_id);
        } else {
            printf("Customer failed to reserve tickets for Showtime %d.\n", showtime_id);
        }
    }

    // Simulate releasing a showtime lock
    int release_showtime_id;
    printf("Enter the showtime ID to release the lock: ");
    scanf("%d", &release_showtime_id);
    releaseShowtime(release_showtime_id);
    printf("Showtime %d lock released manually.\n", release_showtime_id);

    // Simulate the passage of time and check for expired showtime locks
    sleep(TIME_WINDOW + 1);
    checkAndReleaseExpiredLocks();

    return 0;
}
